#include <iostream>

#include <PxPhysicsAPI.h>

using namespace physx;

int main()
{
    PxDefaultAllocator allocator{};
    PxDefaultErrorCallback errorCallback{};

    // create foundation
    const auto foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
    if (!foundation)
    {
        std::cerr << "PxCreateFoundation failed!" << std::endl;
    }

    // create pvd (debugging handle)
    const auto pvd = PxCreatePvd(*foundation);

    // create pvd session, connect.
    const auto transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // create physics
    const auto physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true, pvd);
    if (!physics)
    {
        std::cerr << "PxCreatePhysics failed!" << std::endl;
    }

    // create a scene
    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f); // scene gravity
    auto cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = cpuDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    auto scene = physics->createScene(sceneDesc);
    auto pvdClient = scene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // create a smooth, no friction, 100% bouncing material
    auto smoothMaterial = physics->createMaterial(0.0f, 0.0f, 1.0f);

    // create a plane
    auto groundPlane = PxCreatePlane(*physics, PxPlane(0, 1, 0, 0), *smoothMaterial);
    scene->addActor(*groundPlane);

    // create a sphere shape and rigid dynamics
    PxTransform transform(PxVec3(0.0f, 10.0f, 0.0f));
    PxSphereGeometry sphereGeometry(1.0f);
    auto ball = PxCreateDynamic(*physics, transform, sphereGeometry, *smoothMaterial, 10.0f);
    scene->addActor(*ball);

    // simulate physics world
    for (int i = 0; i < 500; ++i)
    {
        scene->simulate(1.0f / 60.0f);
        scene->fetchResults(true);
    }

    std::cout << "Simulation End!" << std::endl;

    physics->release();
    foundation->release();

    return 0;
}