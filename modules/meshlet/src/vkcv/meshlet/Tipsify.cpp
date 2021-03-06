
#include <vkcv/Logger.hpp>
#include "vkcv/meshlet/Tipsify.hpp"
#include <iostream>

namespace vkcv::meshlet {

    const int maxUsedVertices           = 128;

    /**
     * modulo operation with maxUsedVertices
     * @param number for modulo operation
     * @return number between 0 and maxUsedVertices - 1
     */
    int mod( int number ){
        return (number + maxUsedVertices) % maxUsedVertices;
    }

    /**
     * searches for the next VertexIndex that was used before or returns any vertexIndex if no used was found
     * @param livingVertices vertice that still has a living triangle
     * @param usedVerticeStack stack of vertices that are recently used
     * @param usedVerticeCount number of last used vertices that maxes at @param maxUsedVertices
     * @param usedVerticeOffset the @param usedVerticeStack wraps around and overrides old vertices, this offset keeps the wrapping intact
     * @param vertexCount total vertex count to terminate correctly
     * @param lowestLivingVertexIndex vertexIndex with the lowest number
     * @param currentTriangleIndex index of the currently fanned triangle
     * @param skippedIndices indices that define a new meshlet, even if they are close in memory
     * @return a VertexIndex to be used as fanningVertexIndex
     */
    int skipDeadEnd(
            const std::vector<uint8_t> &livingVertices,
            const std::vector<uint32_t> &usedVerticeStack,
            int &usedVerticeCount,
            int &usedVerticeOffset,
            int vertexCount,
            int &lowestLivingVertexIndex,
            int &currentTriangleIndex,
            std::vector<uint32_t> &skippedIndices) {

        // returns the latest vertex used that has a living triangle
        while (mod(usedVerticeCount) != usedVerticeOffset) {
            // iterate from the latest to the oldest. + maxUsedVertices to always make it a positive number in the range 0 to maxUsedVertices -1
            int nextVertex = usedVerticeStack[mod(--usedVerticeCount)];

            if (livingVertices[nextVertex] > 0) {
                return nextVertex;
            }
        }
        // returns any vertexIndex since no last used has a living triangle
        while (lowestLivingVertexIndex + 1 < vertexCount) {
            lowestLivingVertexIndex++;
            if (livingVertices[lowestLivingVertexIndex] > 0) {
                // add index of the vertex to skippedIndices
                skippedIndices.push_back(static_cast<uint32_t>(currentTriangleIndex * 3));
                return lowestLivingVertexIndex;
            }
        }
        return -1;
    }

    /**
     * searches for the best next candidate as a fanningVertexIndex
     * @param vertexCount total vertexCount of the mesh
     * @param lowestLivingVertexIndex vertexIndex with the lowest number
     * @param cacheSize number to determine in which range vertices are prioritized to me reused
     * @param possibleCandidates candidates for the next vertex to be fanned out
     * @param numPossibleCandidates number of all possible candidates
     * @param lastTimestampCache number of the last iteration the vertex was used
     * @param currentTimeStamp current iteration of all vertices
     * @param livingVertices vertice that still has a living triangle
     * @param usedVerticeStack stack of vertices that are recently used
     * @param usedVerticeCount number of last used vertices that maxes at @param maxUsedVertices
     * @param usedVerticeOffset the @param usedVerticeStack wraps around and overrides old vertices, this offset keeps the wrapping intact
     * @param currentTriangleIndex index of the currently fanned triangle
     * @param skippedIndices indices that define a new meshlet, even if they are close in memory
     * @return a VertexIndex to be used as fanningVertexIndex
     */
    int getNextVertexIndex(int vertexCount,
                           int &lowestLivingVertexIndex,
                           int cacheSize,
                           const std::vector<uint32_t> &possibleCandidates,
                           int numPossibleCandidates,
                           const std::vector<uint32_t> &lastTimestampCache,
                           int currentTimeStamp,
                           const std::vector<uint8_t> &livingVertices,
                           const std::vector<uint32_t> &usedVerticeStack,
                           int &usedVerticeCount,
                           int &usedVerticeOffset,
                           int &currentTriangleIndex,
                           std::vector<uint32_t> &skippedIndices) {
        int nextVertexIndex = -1;
        int maxPriority     = -1;
        // calculates the next possibleCandidates that is recently used
        for (int j = 0; j < numPossibleCandidates; j++) {
            int vertexIndex = possibleCandidates[j];

            // the candidate needs to be not fanned out yet
            if (livingVertices[vertexIndex] > 0) {
                int priority = -1;

                // prioritizes recent used vertices, but tries not to pick one that has many triangles -> fills holes better
                if ( currentTimeStamp - lastTimestampCache[vertexIndex] + 2 * livingVertices[vertexIndex] <=
					 cacheSize) {
                    priority = currentTimeStamp - lastTimestampCache[vertexIndex];
                }
                // select the vertexIndex with the highest priority
                if (priority > maxPriority) {
                    maxPriority     = priority;
                    nextVertexIndex = vertexIndex;
                }
            }
        }

        // if no candidate is alive, try and find another one
        if (nextVertexIndex == -1) {
            nextVertexIndex = skipDeadEnd(
					livingVertices,
					usedVerticeStack,
					usedVerticeCount,
					usedVerticeOffset,
					vertexCount,
					lowestLivingVertexIndex,
					currentTriangleIndex,
					skippedIndices);
        }
        return nextVertexIndex;
    }

    VertexCacheReorderResult tipsifyMesh(
            const std::vector<uint32_t> &indexBuffer32Bit,
            const int vertexCount,
            const unsigned int cacheSize) {

        if (indexBuffer32Bit.empty() || vertexCount <= 0) {
            vkcv_log(LogLevel::ERROR, "Invalid Input.");
            return VertexCacheReorderResult(indexBuffer32Bit , {});
        }
        int triangleCount = indexBuffer32Bit.size() / 3;

       // dynamic array for vertexOccurrence
        std::vector<uint8_t> vertexOccurrence(vertexCount, 0);
        // count the occurrence of a vertex in all among all triangles
        for (size_t i = 0; i < triangleCount * 3; i++) {
            vertexOccurrence[indexBuffer32Bit[i]]++;
        }

        int sum = 0;
        std::vector<uint32_t> offsetVertexOccurrence(vertexCount + 1, 0);
        // highest offset for later iteration
        int maxOffset = 0;
        // calculate the offset of each vertex from the start
        for (int i = 0; i < vertexCount; i++) {
            offsetVertexOccurrence[i]   = sum;
            sum                         += vertexOccurrence[i];

            if (vertexOccurrence[i] > maxOffset) {
                maxOffset = vertexOccurrence[i];
            }
            // reset for reuse
            vertexOccurrence[i] = 0;
        }
        offsetVertexOccurrence[vertexCount] = sum;

        // vertexIndexToTriangle = which vertex belongs to which triangle
        std::vector<uint32_t> vertexIndexToTriangle(3 * triangleCount, 0);
        // vertexOccurrence functions as number of usages in all triangles
        // lowestLivingVertexIndex = number of a triangle
        for (int i = 0; i < triangleCount; i++) {
            // get the pointer to the first vertex of the triangle
            // this allows us to iterate over the indexBuffer with the first vertex of the triangle as start
            const uint32_t *vertexIndexOfTriangle = &indexBuffer32Bit[i * 3];

            vertexIndexToTriangle[offsetVertexOccurrence[vertexIndexOfTriangle[0]] + vertexOccurrence[vertexIndexOfTriangle[0]]] = i;
            vertexOccurrence[vertexIndexOfTriangle[0]]++;

            vertexIndexToTriangle[offsetVertexOccurrence[vertexIndexOfTriangle[1]] + vertexOccurrence[vertexIndexOfTriangle[1]]] = i;
            vertexOccurrence[vertexIndexOfTriangle[1]]++;

            vertexIndexToTriangle[offsetVertexOccurrence[vertexIndexOfTriangle[2]] + vertexOccurrence[vertexIndexOfTriangle[2]]] = i;
            vertexOccurrence[vertexIndexOfTriangle[2]]++;
        }

        // counts if a triangle still uses this vertex
        std::vector<uint8_t>  livingVertices = vertexOccurrence;
        std::vector<uint32_t> lastTimestampCache(vertexCount, 0);

        // stack of already used vertices, if it'currentTimeStamp full it will write to 0 again
        std::vector<uint32_t> usedVerticeStack(maxUsedVertices, 0);

        //currently used vertices
        int usedVerticeCount     = 0;
        // offset if maxUsedVertices was reached and it loops back to 0
        int usedVerticeOffset    = 0;

        // saves if a triangle was emitted (used in the IndexBuffer)
        std::vector<bool> isEmittedTriangles(triangleCount, false);

        // reordered Triangles that get rewritten to the new IndexBuffer
        std::vector<uint32_t> reorderedTriangleIndexBuffer(triangleCount, 0);

        // offset to the latest not used triangleIndex
        int triangleOutputOffset    = 0;
        // vertexIndex to fan out from (fanning VertexIndex)
        int currentVertexIndex      = 0;
        int currentTimeStamp        = cacheSize + 1;
        int lowestLivingVertexIndex = 0;

        std::vector<uint32_t> possibleCandidates(3 * maxOffset);

        int currentTriangleIndex = 0;
        // list of vertex indices where a deadEnd was reached
        // useful to know where the mesh is potentially not contiguous
        std::vector<uint32_t> skippedIndices;

        // run while not all indices are fanned out, -1 equals all are fanned out
        while (currentVertexIndex >= 0) {
            // number of possible candidates for a fanning VertexIndex
            int numPossibleCandidates   = 0;
            // offset of currentVertexIndex and the next VertexIndex
            int startOffset             = offsetVertexOccurrence[currentVertexIndex];
            int endOffset               = offsetVertexOccurrence[currentVertexIndex + 1];
            // iterates over every triangle of currentVertexIndex
            for (int offset = startOffset; offset < endOffset; offset++) {
                int triangleIndex = vertexIndexToTriangle[offset];

                // checks if the triangle is already emitted
                if (!isEmittedTriangles[triangleIndex]) {

                    // get the pointer to the first vertex of the triangle
                    // this allows us to iterate over the indexBuffer with the first vertex of the triangle as start
                    const uint32_t *vertexIndexOfTriangle        = &indexBuffer32Bit[3 * triangleIndex];

                    currentTriangleIndex++;

                    // save emitted vertexIndexOfTriangle to reorderedTriangleIndexBuffer and set it to emitted
                    reorderedTriangleIndexBuffer[triangleOutputOffset++]    = triangleIndex;
                    isEmittedTriangles[triangleIndex]                       = true;

                    // save all vertexIndices of the triangle to reuse as soon as possible
                    for (int j = 0; j < 3; j++) {
                        int vertexIndex = vertexIndexOfTriangle[j];

                        //save vertexIndex to reuseStack
                        usedVerticeStack[mod(usedVerticeCount++)] = vertexIndex;

                        // after looping back increase the start, so it only overrides the oldest vertexIndex
                        if ((mod(usedVerticeCount)) ==
                            (mod(usedVerticeOffset))) {
                            usedVerticeOffset = mod(usedVerticeOffset + 1);
                        }
                        // add vertex to next possibleCandidates as fanning vertex
                        possibleCandidates[numPossibleCandidates++] = vertexIndex;

                        // remove one occurrence of the vertex, since the triangle is used
                        livingVertices[vertexIndex]--;

                        // writes the timestamp (number of iteration) of the last usage, if it wasn't used within the last cacheSize iterations
                        if (currentTimeStamp - lastTimestampCache[vertexIndex] > cacheSize) {
                            lastTimestampCache[vertexIndex] = currentTimeStamp;
                            currentTimeStamp++;
                        }
                    }
                }
            }

            // search for the next vertexIndex to fan out
            currentVertexIndex = getNextVertexIndex(
                    vertexCount, lowestLivingVertexIndex, cacheSize, possibleCandidates, numPossibleCandidates, lastTimestampCache, currentTimeStamp,
                    livingVertices, usedVerticeStack, usedVerticeCount, usedVerticeOffset, currentTriangleIndex, skippedIndices);
        }

        std::vector<uint32_t> reorderedIndexBuffer(3 * triangleCount);

        triangleOutputOffset = 0;
        // rewriting the TriangleIndexBuffer to the new IndexBuffer
        for (int i = 0; i < triangleCount; i++) {
            int triangleIndex = reorderedTriangleIndexBuffer[i];
            // rewriting the triangle index to vertices
            for (int j = 0; j < 3; j++) {
                int vertexIndex = indexBuffer32Bit[(3 * triangleIndex) + j];
                reorderedIndexBuffer[triangleOutputOffset++] = vertexIndex;
            }
        }

        return VertexCacheReorderResult(reorderedIndexBuffer, skippedIndices);
    }
}