#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class MeshPrimitiveType
    {
        None = 0,
        Cube,
        Sphere
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