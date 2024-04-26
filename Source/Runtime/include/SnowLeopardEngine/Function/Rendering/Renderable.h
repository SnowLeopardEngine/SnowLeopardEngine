#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class Renderable
    {
    public:
        explicit Renderable(MeshItem& mesh) : m_Mesh(mesh) {}

        void Render() const { std::cout << m_Mesh.Name << " render" << std::endl; }

    private:
        MeshItem& m_Mesh;
    };
} // namespace SnowLeopardEngine