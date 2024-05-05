#include "SnowLeopardEngine/Function/Asset/OzzMesh.h"

#include "ozz/base/containers/vector_archive.h"
#include "ozz/base/memory/allocator.h"

#include "ozz/base/io/archive.h"

#include "ozz/base/maths/math_archive.h"
#include "ozz/base/maths/simd_math_archive.h"

// NOLINTBEGIN
namespace ozz
{
    namespace io
    {
        void Extern<sample::Mesh::Part>::Save(OArchive& _archive, const sample::Mesh::Part* _parts, size_t _count)
        {
            for (size_t i = 0; i < _count; ++i)
            {
                const sample::Mesh::Part& part = _parts[i];
                _archive << part.positions;
                _archive << part.normals;
                _archive << part.tangents;
                _archive << part.uvs;
                _archive << part.colors;
                _archive << part.joint_indices;
                _archive << part.joint_weights;
            }
        }

        void Extern<sample::Mesh::Part>::Load(IArchive&           _archive,
                                              sample::Mesh::Part* _parts,
                                              size_t              _count,
                                              uint32_t            _version)
        {
            (void)_version;
            for (size_t i = 0; i < _count; ++i)
            {
                sample::Mesh::Part& part = _parts[i];
                _archive >> part.positions;
                _archive >> part.normals;
                _archive >> part.tangents;
                _archive >> part.uvs;
                _archive >> part.colors;
                _archive >> part.joint_indices;
                _archive >> part.joint_weights;
            }
        }

        void Extern<sample::Mesh>::Save(OArchive& _archive, const sample::Mesh* _meshes, size_t _count)
        {
            for (size_t i = 0; i < _count; ++i)
            {
                const sample::Mesh& mesh = _meshes[i];
                _archive << mesh.parts;
                _archive << mesh.triangle_indices;
                _archive << mesh.joint_remaps;
                _archive << mesh.inverse_bind_poses;
            }
        }

        void Extern<sample::Mesh>::Load(IArchive& _archive, sample::Mesh* _meshes, size_t _count, uint32_t _version)
        {
            (void)_version;
            for (size_t i = 0; i < _count; ++i)
            {
                sample::Mesh& mesh = _meshes[i];
                _archive >> mesh.parts;
                _archive >> mesh.triangle_indices;
                _archive >> mesh.joint_remaps;
                _archive >> mesh.inverse_bind_poses;
            }
        }
    } // namespace io
} // namespace ozz
  // NOLINTEND