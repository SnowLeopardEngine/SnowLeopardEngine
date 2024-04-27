#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <fstream>


#include <tiny_obj_loader.h>
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourNavMeshBuilder.h>

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
   

     // 创建上下文对象，用于记录日志和跟踪处理状态
    rcContext* ctx = new rcContext();

    // 初始化导航网格构建的配置结构
    rcConfig cfg;
    memset(&cfg, 0, sizeof(cfg));  // 清零配置，确保从干净的状态开始

    // 设置构建配置参数
    cfg.cs = 0.3f;  // 格子大小
    cfg.ch = 0.2f;  // 格子高度
    cfg.walkableSlopeAngle = 45.0f;  // 最大可行走坡度
    cfg.walkableHeight = static_cast<int>(ceil(2.0f / cfg.ch));  // 通过空间所需的最小高度
    cfg.walkableClimb = static_cast<int>(floor(0.9f / cfg.ch));  // 最大爬升高度
    cfg.walkableRadius = static_cast<int>(ceil(0.6f / cfg.cs));  // 角色半径
    cfg.maxEdgeLen = static_cast<int>(40.0f / cfg.cs);  // 最大边长
    cfg.maxSimplificationError = 1.3f;  // 最大简化误差
    cfg.minRegionArea = static_cast<int>(rcSqr(50));  // 最小区域面积
    cfg.mergeRegionArea = static_cast<int>(rcSqr(20));  // 合并区域的最小面积
    cfg.maxVertsPerPoly = 6;  // 每个多边形的最大顶点数
    cfg.detailSampleDist = cfg.cs * 24.0f;  // 详细网格采样距离
    cfg.detailSampleMaxError = cfg.ch * 1.0f;  // 详细网格的最大采样误差

    // 设置导航网格的边界
    cfg.bmin[0] = cfg.bmin[1] = cfg.bmin[2] = FLT_MAX;
    cfg.bmax[0] = cfg.bmax[1] = cfg.bmax[2] = -FLT_MAX;
    for (int i = 0; i < vertices.size(); i += 3) {
        cfg.bmin[0] = std::min(cfg.bmin[0], vertices[i]);
        cfg.bmin[1] = std::min(cfg.bmin[1], vertices[i + 1]);
        cfg.bmin[2] = std::min(cfg.bmin[2], vertices[i + 2]);
        cfg.bmax[0] = std::max(cfg.bmax[0], vertices[i]);
        cfg.bmax[1] = std::max(cfg.bmax[1], vertices[i + 1]);
        cfg.bmax[2] = std::max(cfg.bmax[2], vertices[i + 2]);
    }

    // 计算网格大小
    rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

    // 创建高度场
    rcHeightfield* solid = rcAllocHeightfield();
    if (!solid || !rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
        throw std::runtime_error("Failed to create heightfield");
    }

    // 光栅化输入的多边形数据
    std::vector<unsigned char> triAreas(indices.size() / 3, 0);
    std::vector<int> intIndices(indices.begin(), indices.end());
    std::vector<unsigned short> ushortIndices(indices.begin(), indices.end());
    rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle, vertices.data(), vertices.size() / 3, intIndices.data(), indices.size() / 3, triAreas.data());
    if (!rcRasterizeTriangles(ctx, vertices.data(), vertices.size() / 3, ushortIndices.data(), triAreas.data(), indices.size() / 3, *solid, cfg.walkableClimb)) {
        throw std::runtime_error("Failed to rasterize triangles");
    }

    // 过滤步行表面，清除不适合步行的区域
    rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *solid);
    rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
    rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *solid);

    // 构建紧凑的高度场
    rcCompactHeightfield* chf = rcAllocCompactHeightfield();
    if (!chf || !rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
        throw std::runtime_error("Failed to build compact heightfield");
    }
   

    // Calculate distance field (for Watershed partitioning)
    if (!rcBuildDistanceField(ctx, *chf)) {
      throw std::runtime_error("Failed to build distance field");
    }

// Partition the heightfield (choose one based on use case)
    if (!rcBuildRegions(ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea)) {
      throw std::runtime_error("Failed to build regions");
    }

// Build contours from the partitioned heightfield
    rcContourSet* cset = rcAllocContourSet();
    if (!cset || !rcBuildContours(ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)) {
    throw std::runtime_error("Failed to build contours");
   }


    // 构建多边形网格
    rcPolyMesh* pmesh = rcAllocPolyMesh();
    if (!pmesh || !rcBuildPolyMesh(ctx, *cset, cfg.maxVertsPerPoly, *pmesh)) {
        throw std::runtime_error("Failed to build poly mesh");
    }

    // 构建详细网格
    rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
    if (!dmesh || !rcBuildPolyMeshDetail(ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh)) {
        throw std::runtime_error("Failed to build detail mesh");
    }

    // 创建 Detour 导航数据
    unsigned char* navData = nullptr;
    int navDataSize = 0;
    dtNavMeshCreateParams params{};
    memset(&params, 0, sizeof(params));
    params.verts = pmesh->verts;
    params.vertCount = pmesh->nverts;
    params.polys = pmesh->polys;
    params.polyAreas = pmesh->areas;
    params.polyFlags = pmesh->flags;
    params.polyCount = pmesh->npolys;
    params.nvp = pmesh->nvp;
    params.detailMeshes = dmesh->meshes;
    params.detailVerts = dmesh->verts;
    params.detailVertsCount = dmesh->nverts;
    params.detailTris = dmesh->tris;
    params.detailTriCount = dmesh->ntris;
    params.walkableHeight = (float)cfg.walkableHeight;
    params.walkableRadius = (float)cfg.walkableRadius;
    params.walkableClimb = (float)cfg.walkableClimb;
    params.bmin[0] = cfg.bmin[0];
    params.bmin[1] = cfg.bmin[1];
    params.bmin[2] = cfg.bmin[2];
    params.bmax[0] = cfg.bmax[0];
    params.bmax[1] = cfg.bmax[1];
    params.bmax[2] = cfg.bmax[2];
    params.cs = cfg.cs;
    params.ch = cfg.ch;
    params.buildBvTree = true;

    if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
        throw std::runtime_error("Failed to create Detour navmesh data");
    }

    // 保存二进制数据到文件
    std::ofstream file("navmesh.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(navData), navDataSize);
    file.close();

    // 清理
    dtFree(navData);
    rcFreePolyMeshDetail(dmesh);
    rcFreePolyMesh(pmesh);
    rcFreeCompactHeightfield(chf);
    rcFreeHeightField(solid);
    delete ctx;
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
