#include "vkcv/meshlet/Forsyth.hpp"
#include <vkcv/Logger.hpp>
#include <array>
#include <cmath>

namespace vkcv::meshlet
{
    /*
     * CACHE AND VALENCE
     * SIZE AND SCORE CONSTANTS
     * CHANGE AS NEEDED
     */

    // set these to adjust performance and result quality
    const size_t VERTEX_CACHE_SIZE = 8;
    const size_t CACHE_FUNCTION_LENGTH = 32;

    // score function constants
    const float CACHE_DECAY_POWER = 1.5f;
    const float LAST_TRI_SCORE = 0.75f;

    const float VALENCE_BOOST_SCALE = 2.0f;
    const float VALENCE_BOOST_POWER = 0.5f;

    // sizes for precalculated tables
    // make sure that cache score is always >= vertex_cache_size
    const size_t CACHE_SCORE_TABLE_SIZE = 32;
    const size_t VALENCE_SCORE_TABLE_SIZE = 32;

    // precalculated tables
    std::array<float, CACHE_SCORE_TABLE_SIZE> cachePositionScore = {};
    std::array<float, VALENCE_SCORE_TABLE_SIZE> valenceScore = {};

    // function to populate the cache position and valence score tables
    void initScoreTables()
    {
        for(size_t i = 0; i < CACHE_SCORE_TABLE_SIZE; i++)
        {
            float score = 0.0f;
            if (i < 3)
            {
                score = LAST_TRI_SCORE;
            }
            else
            {
                const float scaler = 1.0f / static_cast<float>(CACHE_FUNCTION_LENGTH - 3);
                score = 1.0f - (i - 3) * scaler;
                score = std::pow(score, CACHE_DECAY_POWER);
            }
            cachePositionScore[i] = score;
        }

        for(size_t i = 0; i < VALENCE_SCORE_TABLE_SIZE; i++)
        {
            const float valenceBoost = std::pow(i, -VALENCE_BOOST_POWER);
            const float score = VALENCE_BOOST_SCALE * valenceBoost;

            valenceScore[i] = score;
        }
    }

    /**
     * Return the vertex' score, depending on its current active triangle count and cache position
     * Add a valence boost to score, if active triangles are below VALENCE_SCORE_TABLE_SIZE
     * @param numActiveTris the active triangles on this vertex
     * @param cachePos the vertex' position in the cache
     * @return vertex' score
     */
    float findVertexScore(uint32_t numActiveTris, int32_t cachePos)
    {
        if(numActiveTris == 0)
            return 0.0f;

        float score = 0.0f;

        if (cachePos >= 0)
            score = cachePositionScore[cachePos];

        if (numActiveTris < VALENCE_SCORE_TABLE_SIZE)
            score += valenceScore[numActiveTris];

        return score;
    }

    VertexCacheReorderResult forsythReorder(const std::vector<uint32_t> &idxBuf, const size_t vertexCount)
    {
        std::vector<uint32_t> skippedIndices;

        initScoreTables();

        // get the total triangle count from the index buffer
        const size_t triangleCount = idxBuf.size() / 3;

        // per-vertex active triangle count
        std::vector<uint8_t> numActiveTris(vertexCount, 0);
        // iterate over indices, count total occurrences of each vertex
        for(const auto index : idxBuf)
        {
            if(numActiveTris[index] == UINT8_MAX)
            {
                vkcv_log(LogLevel::ERROR, "Unsupported mesh.");
                vkcv_log(LogLevel::ERROR, "Vertex shared by too many triangles.");
                return VertexCacheReorderResult({}, {});
            }

            numActiveTris[index]++;
        }


        // allocate remaining vectors
        /**
         * offsets: contains the vertices' offset into the triangleIndices vector
         * Offset itself is the sum of triangles required by the previous vertices
         *
         * lastScore: the vertices' most recent calculated score
         *
         * cacheTag: the vertices' most recent cache score
         *
         * triangleAdded: boolean flags to denote whether a triangle has been processed or not
         *
         * triangleScore: total score of the three vertices making up the triangle
         *
         * triangleIndices: indices for the triangles
         */
        std::vector<uint32_t> offsets(vertexCount, 0);
        std::vector<float> lastScore(vertexCount, 0.0f);
        std::vector<int8_t> cacheTag(vertexCount, -1);

        std::vector<bool> triangleAdded(triangleCount, false);
        std::vector<float> triangleScore(triangleCount, 0.0f);

        std::vector<int32_t> triangleIndices(idxBuf.size(), 0);


        // sum the number of active triangles for all previous vertices
        // null the number of active triangles afterwards for recalculation in second loop
        uint32_t sum = 0;
        for(size_t i = 0; i < vertexCount; i++)
        {
            offsets[i] = sum;
            sum += numActiveTris[i];
            numActiveTris[i] = 0;
        }
        // create the triangle indices, using the newly calculated offsets, and increment numActiveTris
        // every vertex should be referenced by a triangle index now
        for(size_t i = 0; i < triangleCount; i++)
        {
            for(size_t j = 0; j < 3; j++)
            {
                uint32_t v = idxBuf[3 * i + j];
                triangleIndices[offsets[v] + numActiveTris[v]] = static_cast<int32_t>(i);
                numActiveTris[v]++;
            }
        }

        // calculate and initialize the triangle score, by summing the vertices' score
        for (size_t i = 0; i < vertexCount; i++)
        {
            lastScore[i] = findVertexScore(numActiveTris[i], static_cast<int32_t>(cacheTag[i]));

            for(size_t j = 0; j < numActiveTris[i]; j++)
            {
                triangleScore[triangleIndices[offsets[i] + j]] += lastScore[i];
            }
        }

        // find best triangle to start reordering with
        int32_t bestTriangle = -1;
        float   bestScore    = -1.0f;
        for(size_t i = 0; i < triangleCount; i++)
        {
            if(triangleScore[i] > bestScore)
            {
                bestScore = triangleScore[i];
                bestTriangle = static_cast<int32_t>(i);
            }
        }

        // allocate output triangles
        std::vector<int32_t> outTriangles(triangleCount, 0);
        uint32_t outPos = 0;

        // initialize cache (with -1)
        std::array<int32_t, VERTEX_CACHE_SIZE + 3> cache = {};
        for(auto &element : cache)
        {
            element = -1;
        }

        uint32_t scanPos = 0;

        // begin reordering routine
        // output the currently best triangle, as long as there are triangles left to output
        while(bestTriangle >= 0)
        {
            // mark best triangle as added
            triangleAdded[bestTriangle] = true;
            // output this triangle
            outTriangles[outPos++] = bestTriangle;

            // push best triangle's vertices into the cache
            for(size_t i = 0; i < 3; i++)
            {
                uint32_t v = idxBuf[3 * bestTriangle + i];

                // get vertex' cache position, if its -1, set its position to the end
                int8_t endPos = cacheTag[v];
                if(endPos < 0)
                    endPos = static_cast<int8_t>(VERTEX_CACHE_SIZE + i);

                // shift vertices' cache entries forward by one
                for(int8_t j = endPos; j > i; j--)
                {
                    cache[j] = cache[j - 1];

                    // if cache slot is valid vertex,
                    // update the vertex cache tag accordingly
                    if (cache[j] >= 0)
                        cacheTag[cache[j]]++;
                }

                // insert current vertex into its new target slot
                cache[i] = static_cast<int32_t>(v);
                cacheTag[v] = static_cast<int8_t>(i);

                // find current triangle in the list of active triangles
                // remove it by moving the last triangle into the slot the current triangle is holding.
                for (size_t j = 0; j < numActiveTris[v]; j++)
                {
                    if(triangleIndices[offsets[v] + j] == bestTriangle)
                    {
                        triangleIndices[offsets[v] + j] = triangleIndices[offsets[v] + numActiveTris[v] - 1];
                        break;
                    }
                }
                // shorten the list
                numActiveTris[v]--;
            }

            // update scores of all triangles in cache
            for (size_t i = 0; i < cache.size(); i++)
            {
                int32_t v = cache[i];
                if (v < 0)
                    break;

                // this vertex has been pushed outside of the actual cache
                if(i >= VERTEX_CACHE_SIZE)
                {
                    cacheTag[v] = -1;
                    cache[i] = -1;
                }

                float newScore = findVertexScore(numActiveTris[v], cacheTag[v]);
                float diff = newScore - lastScore[v];

                for(size_t j = 0; j < numActiveTris[v]; j++)
                {
                    triangleScore[triangleIndices[offsets[v] + j]] += diff;
                }
                lastScore[v] = newScore;
            }

            // find best triangle reference by vertices in cache
            bestTriangle = -1;
            bestScore = -1.0f;
            for(size_t i = 0; i < VERTEX_CACHE_SIZE; i++)
            {
                if (cache[i] < 0)
                    break;

                int32_t v = cache[i];
                for(size_t j = 0; j < numActiveTris[v]; j++)
                {
                    int32_t t = triangleIndices[offsets[v] + j];
                    if(triangleScore[t] > bestScore)
                    {
                        bestTriangle = t;
                        bestScore = triangleScore[t];
                    }
                }
            }

            // if no triangle was found at all, continue scanning whole list of triangles
            if (bestTriangle < 0)
            {
                for(; scanPos < triangleCount; scanPos++)
                {
                    if(!triangleAdded[scanPos])
                    {
                        bestTriangle = scanPos;

                        skippedIndices.push_back(3 * outPos);

                        break;
                    }
                }
            }
        }


        // convert triangle index array into full triangle list
        std::vector<uint32_t> outIndices(idxBuf.size(), 0);
        outPos = 0;
        for(size_t i = 0; i < triangleCount; i++)
        {
            int32_t t = outTriangles[i];
            for(size_t j = 0; j < 3; j++)
            {
                int32_t v = idxBuf[3 * t + j];
                outIndices[outPos++] = static_cast<uint32_t>(v);
            }
        }

        return VertexCacheReorderResult(outIndices, skippedIndices);
    }
}