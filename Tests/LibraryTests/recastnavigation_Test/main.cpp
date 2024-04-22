#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include <tiny_obj_loader.h>
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

// Load OBJ model using tinyobjloader
bool loadObj(const std::string& filename, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        throw std::runtime_error("Failed to load OBJ file: " + err);
        return false;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            vertices.push_back(attrib.vertices[3 * index.vertex_index]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);
            indices.push_back(static_cast<unsigned int>(indices.size()));
        }
    }

    return true;
}

// Generate NavMesh using Recastnavigation
void generateNavMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    rcContext* ctx = new rcContext();
    rcConfig cfg;
    rcHeightfield* solid = nullptr;
    rcCompactHeightfield* chf = nullptr;
    rcContourSet* cset = nullptr;
    rcPolyMesh* mesh = nullptr;
    rcPolyMeshDetail* dmesh = nullptr;
    unsigned char* triAreas = nullptr;
    rcHeightfieldLayerSet* lset = nullptr;

    memset(&cfg, 0, sizeof(cfg));

    // Set configuration settings
    cfg.cs = 0.3f; // Cell size
    cfg.ch = 0.2f; // Cell height
    cfg.walkableSlopeAngle = 45.0f; // Max slope
    cfg.walkableHeight = static_cast<int>(ceil(2.0f / cfg.ch)); // Agent height
    cfg.walkableClimb = static_cast<int>(floor(0.9f / cfg.ch)); // Agent max step
    cfg.walkableRadius = static_cast<int>(ceil(0.6f / cfg.cs)); // Agent radius
    cfg.maxEdgeLen = static_cast<int>(12 / cfg.cs); // Max edge length
    cfg.maxSimplificationError = 1.3f; // Simplification factor
    cfg.minRegionArea = static_cast<int>(rcSqr(8)); // Min region area
    cfg.mergeRegionArea = static_cast<int>(rcSqr(20)); // Merge region area
    cfg.maxVertsPerPoly = 6; // Max verts per poly
    cfg.detailSampleDist = cfg.cs * 6; // Detail mesh sample distance
    cfg.detailSampleMaxError = cfg.ch * 1.0f; // Detail mesh sample max error

    // Create heightfield
    solid = rcAllocHeightfield();
    if (!solid) {
        throw std::runtime_error("Failed to allocate heightfield");
        return;
    }

    if (!rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
        throw std::runtime_error("Failed to create heightfield");
        return;
    }

    // TODO: Ruofan He Generate NavMesh binary file

    // Cleanup
    delete ctx;
    delete[] triAreas;
    delete lset;
    delete dmesh;
    delete mesh;
    delete cset;
    delete chf;
    delete solid;
}

int main() {
    std::string objFilename = "assets/nav_test.obj";

    // Load OBJ model
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    try {
        loadObj(objFilename, vertices, indices);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Generate NavMesh
    generateNavMesh(vertices, indices);

    std::cout << "NavMesh generated successfully!" << std::endl;

    return 0;
}
