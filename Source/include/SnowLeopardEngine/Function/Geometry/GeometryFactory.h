#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class MeshPrimitiveType
    {
        None = 0,
        Cube,
        Sphere,
        Capsule
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
                {{-side, -side, -side}, {0, -1, 0}, {0, 0}}, // Swap Y and Z
                {{-side, side, -side}, {0, -1, 0}, {0, 1}},  // Swap Y and Z
                {{side, side, -side}, {0, -1, 0}, {1, 1}},   // Swap Y and Z
                {{side, -side, -side}, {0, -1, 0}, {1, 0}},  // Swap Y and Z

                {{-side, -side, side}, {0, 1, 0}, {0, 0}}, // Swap Y and Z
                {{-side, side, side}, {0, 1, 0}, {0, 1}},  // Swap Y and Z
                {{side, side, side}, {0, 1, 0}, {1, 1}},   // Swap Y and Z
                {{side, -side, side}, {0, 1, 0}, {1, 0}},  // Swap Y and Z

                {{-side, side, -side}, {0, 0, -1}, {0, 0}},  {{-side, side, side}, {0, 0, -1}, {0, 1}},
                {{side, side, side}, {0, 0, -1}, {1, 1}},    {{side, side, -side}, {0, 0, -1}, {1, 0}},

                {{-side, -side, -side}, {0, 0, 1}, {0, 0}},  {{-side, -side, side}, {0, 0, 1}, {0, 1}},
                {{side, -side, side}, {0, 0, 1}, {1, 1}},    {{side, -side, -side}, {0, 0, 1}, {1, 0}},

                {{-side, -side, -side}, {-1, 0, 0}, {0, 0}}, {{-side, -side, side}, {-1, 0, 0}, {0, 1}},
                {{-side, side, side}, {-1, 0, 0}, {1, 1}},   {{-side, side, -side}, {-1, 0, 0}, {1, 0}},

                {{side, -side, -side}, {1, 0, 0}, {0, 0}},   {{side, -side, side}, {1, 0, 0}, {0, 1}},
                {{side, side, side}, {1, 0, 0}, {1, 1}},     {{side, side, -side}, {1, 0, 0}, {1, 0}}};

            // Define indices for a unit cube
            cubeData.Indices = {
                0,  1,  2,  0,  2,  3,  // Front
                4,  5,  6,  4,  6,  7,  // Back
                8,  9,  10, 8,  10, 11, // Top
                12, 13, 14, 12, 14, 15, // Bottom
                16, 17, 18, 16, 18, 19, // Left
                20, 21, 22, 20, 22, 23  // Right
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

                    float x = radius * std::sin(theta) * std::cos(phi);
                    float z = radius * std::sin(theta) * std::sin(phi); // Swap y and z
                    float y = radius * std::cos(theta);                 // Swap y and z

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