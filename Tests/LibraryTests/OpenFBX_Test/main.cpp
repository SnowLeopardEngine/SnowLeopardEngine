#include <fstream>
#include <iostream>
#include <ofbx.h>
#include <string>

bool SaveAsOBJ(ofbx::IScene& scene, const std::string& path)
{
    std::ofstream file(path, std::ios::out);
    if (!file.is_open())
    {
        return false;
    }

    int indicesOffset = 0;
    int meshCount     = scene.getMeshCount();

    // output unindexed geometry
    for (int meshIdx = 0; meshIdx < meshCount; ++meshIdx)
    {
        const ofbx::Mesh&          mesh      = *scene.getMesh(meshIdx);
        const ofbx::GeometryData&  geom      = mesh.getGeometryData();
        const ofbx::Vec3Attributes positions = geom.getPositions();
        const ofbx::Vec3Attributes normals   = geom.getNormals();
        const ofbx::Vec2Attributes uvs       = geom.getUVs();

        // each ofbx::Mesh can have several materials == partitions
        for (int partitionIdx = 0; partitionIdx < geom.getPartitionCount(); ++partitionIdx)
        {
            file << "o obj" << meshIdx << "_" << partitionIdx << "\ng grp" << meshIdx << "\n";
            const ofbx::GeometryPartition& partition = geom.getPartition(partitionIdx);

            // partitions most likely have several polygons, they are not triangles necessarily, use ofbx::triangulate
            // if you want triangles
            for (int polygonIdx = 0; polygonIdx < partition.polygon_count; ++polygonIdx)
            {
                const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygonIdx];

                for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
                {
                    ofbx::Vec3 v = positions.get(i);
                    file << "v " << v.x << " " << v.y << " " << v.z << "\n";
                }

                bool hasNormals = normals.values != nullptr;
                if (hasNormals)
                {
                    // normals.indices might be different than positions.indices
                    // but normals.get(i) is normal for positions.get(i)
                    for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
                    {
                        ofbx::Vec3 n = normals.get(i);
                        file << "vn " << n.x << " " << n.y << " " << n.z << "\n";
                    }
                }

                bool hasUvs = uvs.values != nullptr;
                if (hasUvs)
                {
                    for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
                    {
                        ofbx::Vec2 uv = uvs.get(i);
                        file << "vt " << uv.x << " " << uv.y << "\n";
                    }
                }
            }

            for (int polygonIdx = 0; polygonIdx < partition.polygon_count; ++polygonIdx)
            {
                const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygonIdx];
                file << "f ";
                for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
                {
                    file << 1 + i + indicesOffset << " ";
                }
                file << "\n";
            }

            indicesOffset += positions.count;
        }
    }

    file.close();
    return true;
}

ofbx::IScene* LoadFBX(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open the file." << std::endl;
        return nullptr;
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[fileSize];

    file.read(buffer, fileSize);

    // use openfbx to load a scene
    // Ignoring certain nodes will only stop them from being processed not tokenised (i.e. they will still be in the
    // tree)
    ofbx::LoadFlags flags =
        //		ofbx::LoadFlags::IGNORE_MODELS |
        ofbx::LoadFlags::IGNORE_BLEND_SHAPES | ofbx::LoadFlags::IGNORE_CAMERAS | ofbx::LoadFlags::IGNORE_LIGHTS |
        //		ofbx::LoadFlags::IGNORE_TEXTURES |
        ofbx::LoadFlags::IGNORE_SKIN | ofbx::LoadFlags::IGNORE_BONES | ofbx::LoadFlags::IGNORE_PIVOTS |
        //		ofbx::LoadFlags::IGNORE_MATERIALS |
        ofbx::LoadFlags::IGNORE_POSES | ofbx::LoadFlags::IGNORE_VIDEOS | ofbx::LoadFlags::IGNORE_LIMBS |
        //		ofbx::LoadFlags::IGNORE_MESHES |
        ofbx::LoadFlags::IGNORE_ANIMATIONS;

    auto scene = ofbx::load((ofbx::u8*)buffer, fileSize, (ofbx::u16)flags);

    file.close();

    delete[] buffer;

    return scene;
}

int main()
{
    auto scene = LoadFBX("assets/Walking.fbx");
    bool ok    = SaveAsOBJ(*scene, "Walking.obj");
    if (!ok)
    {
        std::cerr << "Failed to save the fbx file as an obj file!" << std::endl;
        return 1;
    }
    std::cout << "Successfully loaded the fbx file and saved it as an obj file!" << std::endl;
    return 0;
}