#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class MeshPrimitiveType : uint8_t
    {
        None = 0,
        Cube,
        Sphere,
        Capsule,
        Heightfield
    };

    struct CubeMesh
    {
        static const MeshPrimitiveType Type = MeshPrimitiveType::Cube;

        static MeshItem Create(float l = 1.0f)
        {
            MeshItem cubeMesh;
            cubeMesh.Name = "Cube";

            MeshData cubeData;

            // Define vertices for a unit cube
            const float side = l / 2.0f;

            cubeData.Vertices = {
                {{-side, -side, -side}, {0, 0, -1}, {0, 0}}, {{-side, side, -side}, {0, 0, -1}, {0, 1}},
                {{side, side, -side}, {0, 0, -1}, {1, 1}},   {{side, -side, -side}, {0, 0, -1}, {1, 0}},

                {{-side, -side, side}, {0, 0, 1}, {0, 0}},   {{-side, side, side}, {0, 0, 1}, {0, 1}},
                {{side, side, side}, {0, 0, 1}, {1, 1}},     {{side, -side, side}, {0, 0, 1}, {1, 0}},

                {{-side, side, -side}, {0, 1, 0}, {0, 0}},   {{-side, side, side}, {0, 1, 0}, {0, 1}},
                {{side, side, side}, {0, 1, 0}, {1, 1}},     {{side, side, -side}, {0, 1, 0}, {1, 0}},

                {{-side, -side, -side}, {0, -1, 0}, {0, 0}}, {{-side, -side, side}, {0, -1, 0}, {0, 1}},
                {{side, -side, side}, {0, -1, 0}, {1, 1}},   {{side, -side, -side}, {0, -1, 0}, {1, 0}},

                {{-side, -side, -side}, {-1, 0, 0}, {0, 0}}, {{-side, -side, side}, {-1, 0, 0}, {0, 1}},
                {{-side, side, side}, {-1, 0, 0}, {1, 1}},   {{-side, side, -side}, {-1, 0, 0}, {1, 0}},

                {{side, -side, -side}, {1, 0, 0}, {0, 0}},   {{side, -side, side}, {1, 0, 0}, {0, 1}},
                {{side, side, side}, {1, 0, 0}, {1, 1}},     {{side, side, -side}, {1, 0, 0}, {1, 0}},
            };

            // Define indices for a unit cube
            cubeData.Indices = {
                0,  1,  2,  0,  2,  3,  // Front
                4,  6,  5,  4,  7,  6,  // Back
                8,  9,  10, 8,  10, 11, // Top
                12, 14, 13, 14, 12, 15, // Bottom
                16, 17, 18, 16, 18, 19, // Left
                20, 22, 21, 22, 20, 23  // Right
            };

            cubeMesh.Data = cubeData;
            return cubeMesh;
        }
    };

    struct SphereMesh
    {
        static const MeshPrimitiveType Type = MeshPrimitiveType::Sphere;

        static MeshItem Create(uint32_t segments = 30, float radius = 0.5f)
        {
            MeshItem sphereMesh;
            sphereMesh.Name = "Sphere";

            MeshData sphereData;

            for (int lat = 0; lat <= segments; ++lat)
            {
                for (int lon = 0; lon <= segments; ++lon)
                {
                    float theta = lat * (M_PI / segments);
                    float phi   = lon * (2 * M_PI / segments);

                    float x = -radius * std::sin(theta) * std::cos(phi);
                    float y = radius * std::cos(theta);
                    float z = radius * std::sin(theta) * std::sin(phi);

                    glm::vec3 position(x, y, z);
                    glm::vec3 normal = glm::normalize(position);
                    glm::vec2 texCoord(static_cast<float>(lon) / segments, static_cast<float>(lat) / segments);

                    sphereData.Vertices.push_back({position, normal, texCoord});
                }
            }

            for (int lat = 0; lat < segments; ++lat)
            {
                for (int lon = 0; lon < segments; ++lon)
                {
                    int current = lat * (segments + 1) + lon;
                    int next    = current + segments + 1;

                    sphereData.Indices.push_back(current);
                    sphereData.Indices.push_back(next);
                    sphereData.Indices.push_back(current + 1);

                    sphereData.Indices.push_back(current + 1);
                    sphereData.Indices.push_back(next);
                    sphereData.Indices.push_back(next + 1);
                }
            }

            sphereMesh.Data = sphereData;
            return sphereMesh;
        }
    };

    struct CapsuleMesh
    {
        static const MeshPrimitiveType Type = MeshPrimitiveType::Capsule;

        static MeshItem Create(float    radius         = 0.5f,
                               float    height         = 2.0f,
                               uint32_t radialSegments = 20,
                               uint32_t heightSegments = 10,
                               uint32_t ringCount      = 10)
        {
            MeshItem capsuleMesh;
            capsuleMesh.Name = "Capsule";

            MeshData                 capsuleData;
            std::vector<VertexData>& vertices = capsuleData.Vertices;
            std::vector<uint32_t>&   indices  = capsuleData.Indices;

            // Calculate the actual height excluding the caps
            float cylinderHeight = height - 2 * radius;
            if (cylinderHeight < 0)
            {
                cylinderHeight = 0;
            }

            // Top Hemisphere
            for (uint32_t lat = 0; lat <= ringCount / 2; ++lat)
            {
                for (uint32_t lon = 0; lon <= radialSegments; ++lon)
                {
                    float theta = lat * M_PI / ringCount - M_PI / 2.0;
                    float phi   = lon * 2 * M_PI / radialSegments;

                    float x = radius * cos(theta) * cos(phi);
                    float y = radius * sin(theta);
                    float z = radius * cos(theta) * sin(phi);

                    glm::vec3 position(x, y + cylinderHeight / 2 + radius, z);
                    glm::vec3 normal = glm::normalize(position - glm::vec3(0, y + cylinderHeight / 2, 0));
                    glm::vec2 texCoord(static_cast<float>(lon) / radialSegments, static_cast<float>(lat) / ringCount);

                    vertices.push_back({position, normal, texCoord});
                }
            }

            // Cylinder
            for (uint32_t i = 0; i <= heightSegments; ++i)
            {
                float v = static_cast<float>(i) / heightSegments;
                float y = v * cylinderHeight - cylinderHeight / 2;

                for (uint32_t j = 0; j <= radialSegments; ++j)
                {
                    float u = static_cast<float>(j) / radialSegments;

                    float theta = u * 2 * M_PI;
                    float x     = radius * cos(theta);
                    float z     = radius * sin(theta);

                    glm::vec3 position(x, y, z);
                    glm::vec3 normal(cos(theta), 0, sin(theta));
                    glm::vec2 texCoord(u, 1 - v);

                    vertices.push_back({position, normal, texCoord});
                }
            }

            // Bottom Hemisphere
            for (uint32_t lat = ringCount / 2; lat <= ringCount; ++lat)
            {
                for (uint32_t lon = 0; lon <= radialSegments; ++lon)
                {
                    float theta = lat * M_PI / ringCount - M_PI / 2.0;
                    float phi   = lon * 2 * M_PI / radialSegments;

                    float x = radius * cos(theta) * cos(phi);
                    float y = radius * sin(theta);
                    float z = radius * cos(theta) * sin(phi);

                    glm::vec3 position(x, y - cylinderHeight / 2 - radius, z);
                    glm::vec3 normal = glm::normalize(position - glm::vec3(0, y - cylinderHeight / 2, 0));
                    glm::vec2 texCoord(static_cast<float>(lon) / radialSegments, static_cast<float>(lat) / ringCount);

                    vertices.push_back({position, normal, texCoord});
                }
            }

            // Indices
            // Top hemisphere
            for (uint32_t lat = 0; lat < ringCount / 2; ++lat)
            {
                for (uint32_t lon = 0; lon < radialSegments; ++lon)
                {
                    uint32_t first  = lat * (radialSegments + 1) + lon;
                    uint32_t second = first + radialSegments + 1;

                    indices.push_back(first);
                    indices.push_back(second);
                    indices.push_back(first + 1);

                    indices.push_back(first + 1);
                    indices.push_back(second);
                    indices.push_back(second + 1);
                }
            }

            // Cylinder
            uint32_t baseIndex = (ringCount / 2 + 1) * (radialSegments + 1); // Adjust base index for cylinder part
            for (uint32_t i = 0; i < heightSegments; ++i)
            {
                for (uint32_t j = 0; j < radialSegments; ++j)
                {
                    uint32_t first  = (i * (radialSegments + 1)) + j + baseIndex;
                    uint32_t second = first + radialSegments + 1;

                    indices.push_back(first);
                    indices.push_back(second);
                    indices.push_back(first + 1);

                    indices.push_back(first + 1);
                    indices.push_back(second);
                    indices.push_back(second + 1);
                }
            }

            // Bottom hemisphere
            uint32_t bottomBaseIndex =
                baseIndex + (heightSegments + 1) * (radialSegments + 1); // Adjust base index for bottom hemisphere
            for (uint32_t lat = ringCount / 2; lat < ringCount; ++lat)
            {
                for (uint32_t lon = 0; lon < radialSegments; ++lon)
                {
                    uint32_t first  = lat * (radialSegments + 1) + lon + bottomBaseIndex;
                    uint32_t second = first + radialSegments + 1;

                    indices.push_back(first);
                    indices.push_back(second);
                    indices.push_back(first + 1);

                    indices.push_back(first + 1);
                    indices.push_back(second);
                    indices.push_back(second + 1);
                }
            }

            capsuleMesh.Data = capsuleData;
            return capsuleMesh;
        }
    };

    namespace Utils
    {
        static std::vector<std::vector<float>> GenerateBlankHeightMap(int x, int y)
        {
            std::vector<std::vector<float>> heightMap(x, std::vector<float>(y, 0.0f));
            return heightMap;
        }
    } // namespace Utils

    struct HeightfieldMesh
    {
        static const MeshPrimitiveType Type = MeshPrimitiveType::Heightfield;

        static MeshItem Create(const std::vector<std::vector<float>>& heightMap,
                               float                                  scaleX = 1.0f,
                               float                                  scaleY = 1.0f,
                               float                                  scaleZ = 1.0f)
        {
            MeshItem heightfieldMesh;
            heightfieldMesh.Name = "Heightfield";

            MeshData heightfieldData;

            for (size_t row = 0; row < heightMap.size(); ++row)
            {
                for (size_t col = 0; col < heightMap[row].size(); ++col)
                {
                    float x = static_cast<float>(col) * scaleX;
                    float y = heightMap[row][col] * scaleY;
                    float z = static_cast<float>(row) * scaleZ;

                    glm::vec3 position(x, y, z);
                    glm::vec3 normal = CalculateNormal(heightMap, row, col, scaleX, scaleZ);
                    glm::vec2 texCoord(static_cast<float>(col) / heightMap[row].size(),
                                       static_cast<float>(row) / heightMap.size());

                    heightfieldData.Vertices.push_back({position, normal, texCoord});
                }
            }

            for (size_t row = 0; row < heightMap.size() - 1; ++row)
            {
                for (size_t col = 0; col < heightMap[row].size() - 1; ++col)
                {
                    int current = row * heightMap[row].size() + col;
                    int next    = current + heightMap[row].size();

                    heightfieldData.Indices.push_back(current);
                    heightfieldData.Indices.push_back(next);
                    heightfieldData.Indices.push_back(current + 1);

                    heightfieldData.Indices.push_back(current + 1);
                    heightfieldData.Indices.push_back(next);
                    heightfieldData.Indices.push_back(next + 1);
                }
            }

            heightfieldMesh.Data = heightfieldData;
            return heightfieldMesh;
        }

    private:
        static glm::vec3 CalculateNormal(const std::vector<std::vector<float>>& heightMap,
                                         size_t                                 row,
                                         size_t                                 col,
                                         float                                  scaleX,
                                         float                                  scaleZ)
        {
            glm::vec3 normal(0.0f, 1.0f, 0.0f);

            if (row > 0 && row < heightMap.size() - 1 && col > 0 && col < heightMap[row].size() - 1)
            {
                float dzdx = (heightMap[row][col + 1] - heightMap[row][col - 1]) / (2.0f * scaleX);
                float dzdy = (heightMap[row + 1][col] - heightMap[row - 1][col]) / (2.0f * scaleZ);
                normal     = glm::normalize(glm::vec3(-dzdx, 1.0f, dzdy));
            }

            return normal;
        }
    };

    class GeometryFactory
    {
    public:
        template<typename T, typename... Args>
        static MeshItem CreateMeshPrimitive(Args&&... args)
        {
            return T::Create(std::forward<Args>(args)...);
        }
    };
} // namespace SnowLeopardEngine