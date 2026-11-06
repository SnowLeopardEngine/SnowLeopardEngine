// Drops a sphere onto a static floor and checks that it comes to rest on top of it.

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <cmath>
#include <cstdio>
#include <thread>

using namespace JPH;

namespace Layers
{
    static constexpr ObjectLayer NonMoving = 0;
    static constexpr ObjectLayer Moving    = 1;
    static constexpr ObjectLayer Count     = 2;
} // namespace Layers

class BPLayerInterface final : public BroadPhaseLayerInterface
{
public:
    uint             GetNumBroadPhaseLayers() const override { return 2; }
    BroadPhaseLayer  GetBroadPhaseLayer(ObjectLayer inLayer) const override { return BroadPhaseLayer(inLayer); }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
    {
        return inLayer.GetValue() == Layers::NonMoving ? "NonMoving" : "Moving";
    }
#endif
};

class ObjectVsBPFilter final : public ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(ObjectLayer inLayer, BroadPhaseLayer inBPLayer) const override
    {
        return inLayer == Layers::Moving || inBPLayer.GetValue() == Layers::Moving;
    }
};

class ObjectPairFilter final : public ObjectLayerPairFilter
{
public:
    bool ShouldCollide(ObjectLayer a, ObjectLayer b) const override
    {
        return a == Layers::Moving || b == Layers::Moving;
    }
};

int main()
{
    RegisterDefaultAllocator();
    Factory::sInstance = new Factory();
    RegisterTypes();

    bool ok = false;
    {
        TempAllocatorImpl   tempAllocator(10 * 1024 * 1024);
        JobSystemThreadPool jobSystem(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

        BPLayerInterface bpLayers;
        ObjectVsBPFilter objectVsBp;
        ObjectPairFilter objectPair;

        PhysicsSystem physics;
        physics.Init(1024, 0, 1024, 1024, bpLayers, objectVsBp, objectPair);

        BodyInterface& bodies = physics.GetBodyInterface();

        BodyCreationSettings floorSettings(
            new BoxShape(Vec3(50, 1, 50)), RVec3(0, -1, 0), Quat::sIdentity(), EMotionType::Static, Layers::NonMoving);
        bodies.CreateAndAddBody(floorSettings, EActivation::DontActivate);

        BodyCreationSettings sphereSettings(
            new SphereShape(0.5f), RVec3(0, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::Moving);
        BodyID sphere = bodies.CreateAndAddBody(sphereSettings, EActivation::Activate);

        physics.OptimizeBroadPhase();

        for (int step = 0; step < 300 && bodies.IsActive(sphere); ++step)
        {
            physics.Update(1.0f / 60.0f, 1, &tempAllocator, &jobSystem);
        }

        float y = static_cast<float>(bodies.GetCenterOfMassPosition(sphere).GetY());
        std::printf("Sphere resting height: %.3f (expected ~0.5)\n", y);

        ok = std::fabs(y - 0.5f) < 0.05f;
        std::printf(ok ? "JoltPhysics_Test PASSED\n" : "JoltPhysics_Test FAILED\n");
    } // PhysicsSystem and its bodies are destroyed before the types are unregistered

    UnregisterTypes();
    delete Factory::sInstance;
    Factory::sInstance = nullptr;
    return ok ? 0 : 1;
}
