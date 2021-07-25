#include "vkcv/meshlet/Forsyth.hpp"
#include <vkcv/Logger.hpp>
#include <array>
#include <cmath>
#include <iostream>

namespace vkcv::meshlet
{
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
                score = std::powf(score, CACHE_DECAY_POWER);
            }
            cachePositionScore[i] = score;
        }

        for(size_t i = 0; i < VALENCE_SCORE_TABLE_SIZE; i++)
        {
            const float valenceBoost = std::powf(i, -VALENCE_BOOST_POWER);
            const float score = VALENCE_BOOST_SCALE * valenceBoost;

            valenceScore[i] = score;
        }
    }

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

    std::vector<uint32_t> forsythReorder(const std::vector<uint32_t> &idxBuf, const size_t vertexCount)
    {
        initScoreTables();
        /**
        std::cout << "CACHE POSITION SCORES:" << std::endl;
        for(const auto element : cachePositionScore)
            std::cout << element << std::endl;

        std::cout << "VALENCE SCORES:" << std::endl;
        for(const auto element : valenceScore)
            std::cout << element << std::endl;
        **/
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
                return {};
            }

            numActiveTris[index]++;
        }


        // allocate remaining vectors
        std::vector<uint32_t> offsets(vertexCount, 0);
        std::vector<float> lastScore(vertexCount, 0.0f);
        std::vector<int8_t> cacheTag(vertexCount, -1);

        std::vector<bool> triangleAdded(triangleCount, false);
        std::vector<float> triangleScore(triangleCount, 0.0f);

        std::vector<int32_t> triangleIndices(idxBuf.size(), 0);


        // count the triangle array offset for each vertex, initialize the rest of the data.
        // ??????????????????????????
        uint32_t sum = 0;
        for(size_t i = 0; i < vertexCount; i++)
        {
            offsets[i] = sum;
            sum += numActiveTris[i];
            numActiveTris[i] = 0;
        }

        // fill the vertex data structures with indices to the triangles using each vertex
        // ??????????????????????????
        for(size_t i = 0; i < triangleCount; i++)
        {
            for(size_t j = 0; j < 3; j++)
            {
                uint32_t v = idxBuf[3 * i + j];
                triangleIndices[offsets[v] + numActiveTris[v]] = static_cast<int32_t>(i);
                numActiveTris[v]++;
            }
        }

        // init score for all vertices
        // ??????????????????????????
        for (size_t i = 0; i < vertexCount; i++)
        {
            lastScore[i] = findVertexScore(numActiveTris[i], static_cast<int32_t>(cacheTag[i]));

            for(size_t j = 0; j < numActiveTris[i]; j++)
            {
                triangleScore[triangleIndices[offsets[i] + j]] += lastScore[i];
            }
        }

        // find best triangle
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

        // init cache (with -1)
        std::array<int32_t, VERTEX_CACHE_SIZE + 3> cache = {};
        for(auto &element : cache)
        {
            element = -1;
        }

        uint32_t scanPos = 0;

        while(bestTriangle >= 0)
        {
            // mark triangle as added
            triangleAdded[bestTriangle] = true;
            // output triangle
            outTriangles[outPos++] = bestTriangle;

            for(size_t i = 0; i < 3; i++)
            {
                uint32_t v = idxBuf[3 * bestTriangle + i];

                int8_t endPos = cacheTag[v];
                if(endPos < 0)
                    endPos = static_cast<int8_t>(VERTEX_CACHE_SIZE + i);

                for(int8_t j = endPos; j > i; j--)
                {
                    cache[j] = cache[j - 1];

                    if (cache[j] >= 0)
                        cacheTag[cache[j]]++;
                }

                cache[i] = static_cast<int32_t>(v);
                cacheTag[v] = static_cast<int8_t>(i);


                for (size_t j = 0; j < numActiveTris[v]; j++)
                {
                    if(triangleIndices[offsets[v] + j] == bestTriangle)
                    {
                        triangleIndices[offsets[v] + j] = triangleIndices[offsets[v] + numActiveTris[v] - 1];
                        break;
                    }
                }
                numActiveTris[v]--;
            }

            // update scores of all triangles in cache
            for (size_t i = 0; i < cache.size(); i++)
            {
                int32_t v = cache[i];
                if (v < 0)
                    break;

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

        return outIndices;
    }
}