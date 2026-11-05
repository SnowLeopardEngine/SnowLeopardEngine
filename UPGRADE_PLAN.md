# SnowLeopardEngine Upgrade Plan

> Baseline: `main` @ `f1da16b` (2025-03-14). The code on `main` is identical to `dev` (2024-09-06); `main` only adds README edits.
> `non-DSA` is one commit ahead (`57706fd`, macOS build fix touching 11 files). `demo-1` is an early branch with 5 unmerged commits and can be ignored. Every other feature branch is already merged.
> Work happens on the long-lived `revival` branch and is merged back phase by phase.
> Rendering reference: the public [VRI](https://github.com/zzxzzk115/VRI) v0.1.17, plus the author's (private) libvultra `dev-VRI` branch, used as a design reference for the VRI integration, the render graph and the .NET hostfxr host.

---

## 1. Current State

| Item | State |
|---|---|
| Size | Runtime ~9.2k lines, Editor ~2.3k, GameDemo 1k, 25 GLSL / DzShader files |
| Build | xmake + custom package repo `SnowLeopardEngine/xmake-repo@dev` (a fork of xmake-repo adding `scoral` and `sphysx-cross`). C++20 |
| Dependencies | spdlog, fmt 9.1, glfw, glm, entt 3.10.3, assimp, miniaudio, stb, tracy, magic_enum, cereal, shaderc, spirv-cross, cryptopp, fg (skaarj1989/FrameGraph), freetype, ozz-animation 0.14.2, PhysX (sphysx-cross), imgui 1.90.1-docking, Coral (scoral) |
| CI | Windows / Linux / macOS workflows |

### Subsystem Maturity

| Subsystem | Maturity | Notes |
|---|---|---|
| Core (Log / Event / UUID / Time / Math / Reflection) | ★★★★ | Usable. `TypeFactory` reflection is basic; SMeta (`__cppast(smeta_unit)` annotations) only covers some components and has no generator yet |
| Engine / EngineContext | ★★★ | Global `g_EngineContext` singleton; subsystem init order is hard-coded in `Engine::Init`; `LifeTimeComponent` works |
| Window / Input | ★★★ | GLFW only; no gamepad support; keyboard event dispatch left as TODO |
| Scene (EnTT) | ★★★ | ~30 components; cereal serialization; asset loading scattered across `LogicScene.cpp` ("Move to AssetManager" TODOs) |
| Asset / IO | ★★ | CoreUUID asset handles exist (#126) but there is no real AssetManager; `Resources.h` imports only some asset types |
| Rendering | ★★★★ | The most complete part: FrameGraph + deferred pipeline, PBR/IBL, SSAO, Bloom, FXAA, tone mapping, WBOIT, skybox, grass, CSM (`ShadowCascadesBuilder` / `CSM.glsl`, no PCSS), GPU instancing, frustum culling |
| Physics (PhysX) | ★★★ | Rigid bodies, colliders, character controller, triggers, overlaps; no damping / CCD settings, no physics material asset |
| Animation (ozz) | ★★ | Single clip playback and transitions; no blending (BlendJob), blackboard or `.dzanimcon` asset |
| Audio (miniaudio) | ★★ | AudioClip / Source / Listener components exist; 3D audio (#127) unfinished |
| In-game GUI | ★★ | Canvas / Image / Text / Button; text drawn with raw GL (`glBindTextureUnit` in `InGameGUIPass`) |
| Navigation | ☆ | **Not in the engine.** recast only appears in `Tests/LibraryTests/recastnavigation_Test` |
| C# scripting | ☆ | **Effectively absent.** `Coral::HostInstance` is a local variable in the `CSharpScriptingSystem` constructor and is destroyed when it returns; only `Tests.Echo` is ever called; no script component, no bindings |
| Editor | ★★ | Docking layout, Viewport / Inspector / Console / Resource panels; no Hierarchy, menu bar, toolbar, Settings or Preview; hand-written Inspector; Viewport has `TODO: Set RT` |
| Project / packaging | ★★ | `.dzproj` format done; simple game builder done (#139); no game template (#138) |

### Rendering Backend Coupling

- Raw `gl*` calls live in only 3 files: `RenderContext.cpp` (948 lines), `GraphicsContext.cpp` and `InGameGUIPass.cpp`.
- **GL types leak into 16 files** (`Buffer.h`, `Texture.h`, `VertexBuffer.h`, `IndexBuffer.h`, `RenderTypeDef.h`, `PipelineState.h`, `GraphicsPipeline.h`, `FrameGraphBuffer.h`, `Character.h`, `Profiling.h`, `TextureLoader.cpp`, ...). `RenderContext.h` includes `<glad/glad.h>` and engine enums are defined with `GL_*` values (`PrimitiveTopology : GLenum`).
- The API is already Vulkan-shaped (`RenderingInfo` / `AttachmentInfo` / begin-rendering style, FrameGraph, pipeline objects), so most pass code can keep its structure.
- Uses GL 4.5 DSA (22 `glCreate*` / `glNamed*` call sites), which is why macOS needed the `non-DSA` branch. That branch becomes obsolete after VRI.
- Shader path is already GLSL → shaderc → SPIR-V → SPIRV-Cross → GLSL, so feeding SPIR-V directly to a Vulkan-class RHI is a small change.

### Known Issues Not Tracked in GitHub Issues

1. **C# host lifetime bug**: `HostInstance` is a local variable (see above).
2. **VSync hard-coded off** (`RenderSystem.cpp`, `TODO: Configurable vsync`). #91 is closed, but the fixed-step / VSync interaction deserves a re-check.
3. Asset loading scattered in `LogicScene.cpp` (7 × `TODO: Move to AssetManager`).
4. No instancing for skinned meshes (TODOs in the GBuffer, WeightedBlended and Grass passes).
5. `Entity::SetName/SetTag/SetLayer` do not validate; tags/layers are not serialized with the project.
6. FIXMEs at `EditorGUISystem.cpp:121` (commented-out clear) and `LogicScene.cpp:614`.
7. Old pinned dependencies (fmt 9.1.0, entt 3.10.3, imgui 1.90.1) and a forked package repo — the first build will very likely hit package problems.
8. Binary `Tools/*/Coral/Coral.Managed.dll` committed to the repository.

---

## 2. Issue Backlog (21 open, grouped)

Legend: `[ ]` not started, `[~]` partially done (judged against the code).

### Rendering
- [~] **#38 Better Rendering System** (umbrella; #110 and #112 remain)
  - [ ] #110 Static batching with meshoptimizer
  - [~] #112 CSM + PCSS (CSM exists, PCSS does not)
- [ ] #60 Optimize high-poly meshes with meshoptimizer — merge into #110
- [ ] #136 RenderSettingsPanel

### Editor
- [~] **#44 Basic Editor** (umbrella)
  - [ ] Menu bar & toolbar
  - [ ] #72 Hierarchy panel
  - [~] #73 Inspector panel — manual version exists; switch to auto-drawing once SMeta is done
  - [ ] Preview panel
  - [ ] Settings panel
- [ ] #131 Material Inspector
- [ ] #135 NavMesh baking panel

### Animation
- [ ] **#68 Better Animation System** (umbrella; #71 done)
  - [~] #69 Animator Controller — transitions done; blackboard and keyboard switching in the example missing
  - [ ] #70 Animation blending (ozz BlendJob)
- [ ] #134 AnimatorController asset (`.dzanimcon`)

### Physics
- [ ] #133 PhysicsMaterial asset (`.dzphymat`), plus damping / CCD settings at the existing TODOs

### Navigation / Gameplay
- [ ] **#55 Basic Path Finding** (umbrella; #57 and #58 are closed but only exist as a library test)
  - [~] #56 Tag / Layer / CullingMask — CullingMask missing
  - [ ] #59 NavMesh & NavMeshAgent classes
  - [ ] #135 (see Editor)

### Audio
- [~] #127 3D sounds (components exist, spatialization unfinished)
- [~] #3 Basic Audio System — checklist complete, **can be closed**; 3D work continues in #127

### Project / Release
- [ ] #138 Game template

**Housekeeping**: close #3; fold #60 into #110.

---

## 3. Roadmap

Principle: **make it build and run first, then replace the foundations (RHI, scripting), then fill in features.** Each phase is independently deliverable and verifiable.

### Phase 0 — Revive the Build (1–2 days)
- [ ] Get `xmake f -m debug && xmake` working on Windows and record every failing package
- [ ] Package strategy (decision 5): move custom recipes into in-repo `external/packages` (libvultra's approach), keeping a personal xmake-repo as fallback
- [ ] Upgrade dependencies: fmt → 10/11, entt → 3.13+, imgui → current docking branch; fix warnings
- [ ] Repair CI; Windows green at minimum
- [ ] Run every example and GameDemo; record what works as the regression baseline

### Phase 1 — Seal the Rendering Layer for VRI (~1 week)
Goal: OpenGL still runs, but GL exists only inside one backend directory.
- [x] Replace `GL_*`-valued enums with engine-owned enums (`PrimitiveTopology`, `PixelFormat`, `CompareOp`, `BlendFactor`, ...)
- [x] Replace `GLuint` handles in `Buffer` / `Texture` / `GraphicsPipeline` with opaque handles (`RenderHandle`)
- [x] Route `InGameGUIPass`'s raw `glBindTextureUnit` through `RenderContext` (glyphs are now engine `Texture`s)
- [x] Move `RenderContext.cpp` and `GraphicsContext.cpp` into `Rendering/Backend/OpenGL/`; no `glad` in public headers
- [x] Make the GL Tracy zones in `Profiling.h` backend-neutral (`SNOW_LEOPARD_PROFILE_GPU`)
- Acceptance: outside the backend directory, `grep -r "GLuint\|glad" Source` is empty, and every example renders identically
  - [x] grep: only `add_deps("glad")` in `Source/Runtime/xmake.lua` remains
  - [x] Clang syntax check of all 107 Runtime / Editor / GameDemo / Examples translation units passes (with and without `TRACY_ENABLE`)
  - [ ] **Not yet verified**: a Windows build and a visual comparison of the examples (needs Phase 0 on a real machine)
- Still GL-bound outside the backend, by design until Phase 2: GLFW context creation in `GLFWWindow`, and the editor's `imgui_impl_opengl3`

> Value: whatever shape VRI's API takes, the problem is confined to one folder instead of 16 files.

### Phase 2 — OpenGL → VRI (3–5 weeks)

**Reference material**
- VRI is **public**: `zzxzzk115/VRI` v0.1.17, a C API (`<vri/vri.h>`, `VriCoreInterface` / `VriSwapChainInterface` function tables). libvultra enables only the Vulkan backend and ships a patched `vri-vultra` package (swapchain extent forwarding, cube-array feature, calibrated timestamps).
- libvultra's integration layer is thin: `Device` (VriDevice, queue, tables, pipeline cache), `Frame` (**one frame in flight**; `submitAndWait` is the resource lifetime boundary), `Texture` / `Buffer` (whole-resource state tracking + `transition()`).
- Render graph: libvultra has its **own RenderGraph** (explicit import / transient / history / export with automatic VRI barriers) and intentionally avoids the external `fg` library.
- Shaders: Slang → SPIR-V with reflection, cooking and hot reload.
- libvultra's built-in renderer (deferred/forward, OpenPBR subset, IBL, CSM, tone mapping, meshlets) overlaps heavily with SLE's pipeline.

**Key constraint**: SLE is public and libvultra is private. `includes("external/libvultra")` would make SLE unbuildable for everyone else. Default approach: **SLE depends only on public VRI and ports the code it needs from libvultra** (same author, MIT).

- [ ] **2.1 VRI integration**: add the `vri` package (with the `vri-vultra` patch recipe); port `Device` / `Frame` / `Swapchain` / `Texture` / `Buffer` into `Rendering/Backend/VRI/`; present the GLFW window through a VRI swapchain (no GL context)
- [ ] **2.2 Frame graph**: `fg` does **not** handle barriers, while VRI requires explicit synchronization. Options:
  - (a) keep `fg` and call `transition()` manually in each pass — small change, easy to get wrong
  - (b) **port libvultra's RenderGraph** and replace `fg` — barriers and transient reuse come for free. **Recommended**; after Phase 1 the setup/execute structure of each pass carries over
- [ ] **2.3 Shaders — DzShader v3 on ANTLR + Slang**: replace the JSON DzShader with a ShaderLab-style text format (Properties / SubShader / Pass / render states / HLSL-like Slang program blocks), parsed by an ANTLR grammar and compiled by Slang to SPIR-V with reflection. libvultra's `.vshader` (`vshader_lexer.g4` / `vshader_parser.g4`, `shader_parser.cpp`, `shader_compiler.cpp`, last-valid hot reload) is the reference. Pipeline states declared in the shader (`Cull`, `ZTest`, `ZWrite`, `Blend`) map onto the engine-owned enums from Phase 1. Port the 25 GLSL / DzShader files; Vulkan conventions (Y flip, [0,1] depth) are handled once in the shader library
- [ ] **2.4 Pass order**: Blit / Final → GBuffer + DeferredLighting → ShadowPrePass / CSM → Skybox / IBL precompute (CubemapConverter, IBLDataGenerator as compute) → SSAO / Bloom (Down/Upsample, GaussianBlur) / FXAA / ToneMapping → WeightedBlended + TransparencyCompose → Grass → InGameGUI (glyph atlas, no `glBindTextureUnit`)
  - Direct references in libvultra: `builtin_renderer.cpp` (GBuffer / DeferredLighting / Skybox), `shadow_cascades.cpp`, `tone_mapping_pass.cpp`, `environment.cpp` (IBL)
- [ ] **2.5 ImGui**: upgrade the editor to imgui 1.92.x docking with a VRI renderer backend (following libvultra's EditorGui); the Viewport samples a texture exported by the render graph (resolves `TODO: Set RT`)
- [ ] **2.6 Cleanup**: remove `glad`, the GL `GraphicsContext` and the `non-DSA` branch; Tracy GPU zones via VRI timestamps
- Platforms: VRI is validated in libvultra on Windows x64 and Linux x86_64 (Vulkan). **Drop macOS from CI for now**; revisit with VRI Metal / MoltenVK later
- Acceptance: the three RenderingSystem examples (Overview / PBR / Instancing) and OverallShowcase run **with zero Vulkan validation errors** and match GL screenshots (finite-frame + readback, `--frames N --capture`)

### Phase 3 — C# Scripting via .NET hostfxr (1–2 weeks)
Replace Coral / scoral with a direct hostfxr host modelled on libvultra's `dotnet_script.cpp` / `ManagedHost.cs`.

- [ ] **Native host**
  - No nethost link: locate `host/fxr/<highest>/hostfxr.dll` under `DOTNET_ROOT` → `~/.dotnet` → `%ProgramFiles%\dotnet`, load with `LoadLibraryW` / `dlopen`
  - `hostfxr_initialize_for_runtime_config` → `hostfxr_get_runtime_delegate(hdt_load_assembly_and_get_function_pointer)` → `hostfxr_close`
  - CoreCLR is process-wide: keep **one runtime per process** and verify later runtimeconfigs match (also fixes the current `HostInstance` lifetime bug)
  - Entry points via `UnmanagedCallersOnly` (delegate type `-1`), e.g. `SnowLeopardEngine.ManagedHost.Entry.Initialize`
- [ ] **Managed side**
  - Two assemblies: `SnowLeopardEngine.ManagedHost` (bridge, has the runtimeconfig) and `SnowLeopardEngine.Scripting` (user-facing API)
  - User scripts load into an `isCollectible: true` `AssemblyLoadContext` that shares the engine API assembly with the default context (so `Entity` types match)
  - Hot reload saves / restores fields as JSON; entity / resource references are kept as handles
- [ ] **ABI**: only PODs, UTF-8 pointer+length strings and opaque handles cross the boundary, via function tables with `struct_size`. Start with hand-written tables for Entity / Transform / Input / Time / Log / Physics / Audio; decide on code generation after SMeta (Phase 4) to avoid two reflection systems
- [ ] Component: `CSharpScriptComponent` (assembly, class, serialized fields) wired into `LogicScene` start / tick / fixed tick / collision callbacks
- [ ] Build: an xmake rule runs `dotnet build` and copies `*.dll` + `*.runtimeconfig.json`; delete `Tools/*/Coral` and the `scoral` dependency
- [ ] .NET version: follow libvultra to **.NET 10** (update the README's .NET 8 note)
- Acceptance: a new `Examples/CSharpScripting` drives a character from C#, and editing the script at runtime hot-reloads while keeping field values

### Phase 4 — Editor (2–3 weeks)
- [ ] Menu bar / toolbar (new / open / save scene and project; Play / Pause / Stop)
- [ ] #72 Hierarchy (drag-to-parent, context menu, multi-select)
- [ ] SMeta generator → #73 auto-drawn Inspector
- [ ] #131 Material Inspector, #136 RenderSettings, Settings and Preview panels
- [ ] Asset pipeline: a real AssetManager; clear the 7 TODOs in `LogicScene`; drag-and-drop from the Resource panel
- [ ] Gizmos (ImGuizmo already present) and picking via an ID buffer instead of the back-buffer hack in `RenderTypeDef.h`

### Phase 5 — Gameplay Systems (order as needed)
- [ ] Animation: #70 BlendJob, #69 blackboard, #134 `.dzanimcon`, state-machine view in the editor
- [ ] Physics: #133 `.dzphymat`, damping / CCD, cleanup on scene unload (`PhysicsSystem.cpp:83`)
- [ ] Navigation: #59 NavMesh / NavMeshAgent (promote the recast test into the engine), #135 baking panel, #56 CullingMask
- [ ] Audio: #127 3D spatialization (miniaudio supports it); close #3
- [ ] Rendering extras: #112 PCSS, #110 / #60 meshoptimizer, skinned-mesh instancing
- [ ] Input: gamepad support

### Phase 6 — New Demo and Template
- [ ] Rebuild GameDemo with C# scripts and the editor instead of a 1000-line hard-coded `main.cpp`
- [ ] #138 game template, wired into the #139 game builder
- [ ] README: new stack, build instructions, screenshots

---

## 4. Decisions

Defaults in **bold** are what this plan currently follows; change them here if needed.

1. **Relationship with libvultra**:
   - **A. Depend only on public VRI; port code from libvultra** — SLE stays independent and publicly buildable
   - B. Depend on libvultra as a subproject (`includes("external/libvultra")`) so SLE is only the game layer (PhysX / ozz / miniaudio / editor / gameplay). Least code, but requires libvultra to be public, and libvultra's architecture rules (Godot-style node tree) differ from SLE's EnTT component model
   - C. Archive SLE and use its physics / animation / audio work as input to libvultra's "optional extensions" roadmap item
2. **Frame graph**: **port libvultra's RenderGraph** vs. keep `fg`.
3. **Shader language**: ~~Vulkan GLSL first, Slang later~~ → **decided: Slang, with DzShader v3 as a ShaderLab-like syntax parsed by ANTLR** (following libvultra's `.vshader`).
4. **Platforms**: **drop macOS temporarily**.
5. **Packages**: **in-repo `external/packages` recipes** vs. keep maintaining the `SnowLeopardEngine/xmake-repo` fork.
6. **C++ / runtime**: whether to follow libvultra to C++23 + static MSVC runtime (MT). PhysX, ozz, etc. must switch together to avoid LNK2038. Undecided; stays C++20 / MD until Phase 2.
7. **Branching**: **long-lived `revival` branch, one PR per phase**; archive or delete the old (fully merged) feature branches.

## 5. Risks
- Custom packages (`sphysx-cross`, `scoral`) and old pinned versions may stall the first build; Phase 0 duration is uncertain.
- With ~18 render passes, the VRI migration needs a GL-vs-VRI screenshot regression loop or subtle differences will slip through.
- Moving from GL to a Vulkan-class API, the usual failure points are coordinate conventions (Y flip, depth range, cubemap orientation) and synchronization. Carry over the VRI patches and the pitfalls libvultra already documented (swapchain extent, cube-array feature, timestamps on hybrid GPUs).
- VRI is still v0.1.x; pin the version and upgrade in step with libvultra.
- The editor Viewport and ImGui backend both depend on the renderer, so the editor may be unusable during Phase 2. Keep the examples working first and switch the editor last.

---

## 6. Progress Log

| Date | Phase | Change |
|---|---|---|
| 2026-11-05 | — | Analysis and plan written |
| 2026-11-05 | 1 | Cherry-picked the clang portability fix from `non-DSA` (also fixes the missing `RenderContext` declaration in `IBLDataGenerator.h`) |
| 2026-11-05 | 1 | Engine-owned enums / `RenderHandle`; backend-neutral GPU profiling zones; glyphs as engine textures; GL implementation moved to `Backend/OpenGL` |
| 2026-11-05 | 2.3 | Shader decision: DzShader v3 = ANTLR + Slang |
