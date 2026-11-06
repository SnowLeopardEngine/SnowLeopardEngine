#pragma once

#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct Model
    {
        std::filesystem::path SourcePath;

        MeshGroup Meshes;

        std::vector<Ref<AnimationClip>> AnimationClips; // ozz
    };

    // A handle to the model's meshes that also keeps the model alive (shared_ptr aliasing constructor), for
    // MeshFilterComponent::Meshes.
    inline Ref<MeshGroup> ShareMeshes(const Ref<Model>& model)
    {
        return model ? Ref<MeshGroup>(model, &model->Meshes) : nullptr;
    }
} // namespace SnowLeopardEngine