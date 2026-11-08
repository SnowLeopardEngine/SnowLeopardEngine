# SnowLeopardEngine Upgrade Plan

> Baseline: `main` @ `f1da16b` (2025-03-14). The code on `main` is identical to `dev` (2024-09-06); `main` only adds README edits.
> `non-DSA` was one commit ahead (`57706fd`, macOS build fix, cherry-picked here). `demo-1` was an early branch with 5 unmerged commits. Every other feature branch was already merged. The old branches are being removed; only `main`, `dev` and `dev-next` stay (decision 7).
> Work happens on the long-lived **`dev-next`** branch and is merged back phase by phase.
> Rendering reference: the public [VRI](https://github.com/zzxzzk115/VRI) v0.1.17, plus the author's (private) libvultra `dev-VRI` branch, used as a design reference for the VRI integration, the render graph and the .NET hostfxr host.

---

## 1. Current State

| Item | State |
|---|---|
| Size | Runtime ~9.2k lines, Editor ~2.3k, GameDemo 1k, 25 GLSL / DzShader files |
| Build | xmake + custom package repo `SnowLeopardEngine/xmake-repo`, now rebuilt from the owner's `xmake-repo@backup` (upstream 2026-02 + Vultra packages) plus the SLE recipes (`scoral`, `fg`, `fastnoise2`). C++20. **Builds and runs on Windows / MSVC 2022 (Phase 0)** |
| Dependencies | spdlog 1.17 (external fmt), fmt 12.1, SDL3 3.4.0 (shared), glm, entt 3.16.0, assimp, miniaudio, stb, tracy, magic_enum, cereal, freetype, ozz-animation 0.14.2, **Jolt Physics 5.5.0** (replaced PhysX), imgui 1.92.5-docking, Coral (scoral) |
| CI | Windows / Linux workflows (Debug + Release, xmake 3.1.2, package cache with retries, headless Jolt test); all green since 2026-11-08. macOS is a target again (decision 4) but has no CI job yet |

### Subsystem Maturity

| Subsystem | Maturity | Notes |
|---|---|---|
| Core (Log / Event / UUID / Time / Math / Reflection) | ★★★★ | Usable. `TypeFactory` reflection is basic; SMeta (`__cppast(smeta_unit)` annotations) only covers some components and has no generator yet |
| Engine / EngineContext | ★★★ | Global `g_EngineContext` singleton; subsystem init order is hard-coded in `Engine::Init`; `LifeTimeComponent` works |
| Window / Input | ★★★ | **SDL3** (step 2.0 done): window + GL context, positional key mapping, mouse (relative mode for hidden cursors), first-gamepad buttons / axes. Text input events, multiple gamepads and a window icon are still TODO |
| Scene (EnTT) | ★★★ | ~30 components; cereal serialization; asset loading scattered across `LogicScene.cpp` ("Move to AssetManager" TODOs) |
| Asset / IO | ★★ | CoreUUID asset handles exist (#126) but there is no real AssetManager; `Resources.h` imports only some asset types |
| Rendering | ★★★★ | The most complete part: FrameGraph + deferred pipeline, PBR/IBL, SSAO, Bloom, FXAA, tone mapping, WBOIT, skybox, grass, CSM (`ShadowCascadesBuilder` / `CSM.glsl`, no PCSS), GPU instancing, frustum culling |
| Physics (Jolt) | ★★★ | Ported from PhysX to Jolt (decision 8): rigid bodies (sphere / box / capsule / convex mesh / height field), `CharacterVirtual` controller, sensors, ray cast, sphere overlap, mass / damping / CCD. Jolt types stay inside `PhysicsSystem.cpp`. No physics material asset yet |
| Animation (ozz) | ★★ | Single clip playback and transitions; no blending (BlendJob), blackboard or `.dzanimcon` asset |
| Audio (miniaudio) | ★★ | AudioClip / Source / Listener components exist; 3D audio (#127) unfinished |
| In-game GUI | ★ | Canvas / Image / Text / Button only; a naive immediate design with no layout, styling or input focus. **Text rendering is known to be wrong** (owner report; cause not yet investigated). Every glyph is its own texture (no atlas); they became engine textures in Phase 1 |
| Navigation | ☆ | **Not in the engine.** recast only appears in `Tests/LibraryTests/recastnavigation_Test` |
| C# scripting | ☆ | **Effectively absent.** `Coral::HostInstance` is a local variable in the `CSharpScriptingSystem` constructor and is destroyed when it returns; only `Tests.Echo` is ever called; no script component, no bindings |
| Editor | ★★½ | Beyond the old `editor` branch: menu bar (new / open / save scene with native dialogs, entity creation, play controls, panel toggles) and global shortcuts; Hierarchy (tree, multi-select, drag-to-parent keeping world transforms, rename, duplicate, delete, filter); GPU picking from an entity-id buffer, occlusion-aware selection outline, ImGuizmo on world transforms; an editor camera that is not a scene entity (fly / pan / orbit / dolly per second, focus); Inspector with editable name, add / remove component, lights, camera, colliders, mesh / material reload, asset drag-and-drop; Render Settings panel (#136) with statistics and per-pass GPU times; Play / Pause / Stop on a scene copy (verified offline). Not yet: undo / redo, the SMeta auto-drawn Inspector, Material Inspector, Preview, multi-viewport, asset pipeline (runtime models are not saved into scenes) |
| Project / packaging | ★★ | `.dzproj` format done; simple game builder done (#139); no game template (#138) |

### Rendering Backend Coupling

- Raw `gl*` calls live in only 3 files: `RenderContext.cpp` (948 lines), `GraphicsContext.cpp` and `InGameGUIPass.cpp`.
- **GL types leak into 16 files** (`Buffer.h`, `Texture.h`, `VertexBuffer.h`, `IndexBuffer.h`, `RenderTypeDef.h`, `PipelineState.h`, `GraphicsPipeline.h`, `FrameGraphBuffer.h`, `Character.h`, `Profiling.h`, `TextureLoader.cpp`, ...). `RenderContext.h` includes `<glad/glad.h>` and engine enums are defined with `GL_*` values (`PrimitiveTopology : GLenum`).
- The API is already Vulkan-shaped (`RenderingInfo` / `AttachmentInfo` / begin-rendering style, FrameGraph, pipeline objects), so most pass code can keep its structure.
- Uses GL 4.5 DSA (22 `glCreate*` / `glNamed*` call sites), which is why macOS needed the (now removed) `non-DSA` branch. VRI makes it moot.
- Shader path is already GLSL → shaderc → SPIR-V → SPIRV-Cross → GLSL, so feeding SPIR-V directly to a Vulkan-class RHI is a small change.

### Known Issues Not Tracked in GitHub Issues

1. **C# host lifetime bug**: `HostInstance` is a local variable (see above).
2. **VSync hard-coded off** (`RenderSystem.cpp`, `TODO: Configurable vsync`). #91 is closed, but the fixed-step / VSync interaction deserves a re-check.
3. Asset loading scattered in `LogicScene.cpp` (7 × `TODO: Move to AssetManager`).
4. No instancing for skinned meshes (TODOs in the GBuffer, WeightedBlended and Grass passes).
5. `Entity::SetName/SetTag/SetLayer` do not validate; tags/layers are not serialized with the project.
6. FIXMEs at `EditorGUISystem.cpp:121` (commented-out clear) and `LogicScene.cpp:614`.
7. ~~Old pinned dependencies~~ — fmt unpinned (12.1), entt 3.10.3 → 3.16.0, imgui 1.90.1 → 1.92.5-docking (Phase 0).
8. Binary `Tools/*/Coral/Coral.Managed.dll` committed to the repository.
9. ~~Tone mapping and gamma correction are in the wrong order~~ — **fixed**, together with the other lighting bugs found once the output was actually looked at (frame captures, see Phase 0):
   - `ToneMappingPass.frag` ran `linearToGamma` before `toneMapACES` (grey veil, lifted blacks). Now ACES on linear HDR with Narkowicz's 0.6 exposure pre-scale, then gamma. Exposure is a constant for now (TODO: camera / render setting).
   - `Grass.frag` wrote the sRGB grass texture straight into the linear HDR buffer; it is decoded first now.
   - `PBRLighting.glsl` multiplied IBL and point lights by the sun's shadow, so everything in the sun's shadow was pitch black. Only the directional light is shadowed now; `NdotV` is clamped.
   - Materials only set the uniforms they list and GL keeps uniforms per program, so materials inherited `metallic`, `emissive`, `use*Map` from the previous draw (the terrain turned mirror-like). `Material::ResetSurfaceDefaults` runs before every bind in the GBuffer and WeightedBlended passes.
   - Not touched: the old JSON `.dzshader` files (unused by the current renderer; replaced by DzShader v3 in 2.3), and material color constants are still treated as linear (a Unity-style sRGB color convention is a decision for DzShader v3).
10. **Text rendering is wrong** (owner report) and the in-game GUI is too naive to build a game UI on (see Phase 5).
11. ~~Debug builds assert at exit in ozz-animation's allocator (`"Memory leak detected"`) in 7 of 14 apps~~ — **fixed**. Cause: `MeshFilterComponent` owned a raw `MeshGroup*` and only freed it when `UsePrimitive` was set, which the examples never set, so every primitive mesh leaked (each `MeshItem` embeds an `ozz::sample::Mesh`). Now `Meshes` is a `Ref<MeshGroup>`; model meshes are shared with `ShareMeshes(model)`, which keeps the model alive; the material / texture caches own their entries and are emptied in `RenderSystem` shutdown; `Engine::Shutdown` destroys scenes before the systems their resources need. This also removed a use-after-free: two examples freed models (and their GPU buffers) after the render context was gone. Still not done: `g_EngineContext` itself is never deleted, and there is no general leak check beyond ozz's counter.

---

## 2. Issue Backlog (21 open, grouped)

Legend: `[ ]` not started, `[~]` partially done (judged against the code).

### Rendering
- [~] **#38 Better Rendering System** (umbrella; #110 and #112 remain)
  - [ ] #110 Static batching with meshoptimizer
  - [~] #112 CSM + PCSS (CSM exists, PCSS does not)
- [ ] #60 Optimize high-poly meshes with meshoptimizer — merge into #110
- [x] #136 RenderSettingsPanel — renderer settings, statistics and per-pass GPU timings (editor)

### Editor
- [~] **#44 Basic Editor** (umbrella)
  - [x] Menu bar & toolbar
  - [x] #72 Hierarchy panel
  - [~] #73 Inspector panel — manual version exists; switch to auto-drawing once SMeta is done
  - [ ] Preview panel
  - [ ] Settings panel
- [ ] #131 Material Inspector
- [ ] #135 NavMesh baking panel

### Animation
- [ ] **#68 Better Animation System** (umbrella; #71 done)
  - [x] #69 Animator Controller — transitions, bool / float / trigger parameters (blackboard); the AnimationSystem example switches Dance / Walk with keys 1 / 2. Covered by the `AnimationPlayback` engine test
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
- [x] Get `xmake f -m debug && xmake` working on Windows and record every failing package (MSVC 14.44, xmake 3.1.2). Failures found:
  - **The old fork shadowed upstream**: `SnowLeopardEngine/xmake-repo@dev` (2024-09) was a full copy of xmake-repo, so its stale recipes won over the official ones; first hit was `glm` ("links not found" + stale checksum). Fixed by rebuilding the fork from the owner's `xmake-repo@backup` (decision 5)
  - `sphysx-cross`: PhysX object paths reach exactly 260 characters under the package cache and cl.exe cannot write them (`C1083`) even with long paths enabled. Resolved by replacing PhysX with Jolt (decision 8)
  - `ozz-animation`: upstream `has_runtime("MD")` is false for `MDd`, so Debug builds got the static CRT (LNK2038 risk). Recipe patched to accept `MD` and `MDd`
  - `spdlog` 1.17 used its bundled fmt next to the engine's fmt 9.1.0 (`fmt::v9` redefinitions). spdlog now uses `fmt_external`, the fmt pin is gone, and fmt ≥ 11 requires `/utf-8` on MSVC (added globally)
  - fmt ≥ 10 no longer formats enums implicitly: `LogSystem.h` adds a formatter that logs enums by name (magic_enum)
  - `scoral` (Coral): hostfxr lookup only accepted a `host/fxr` folder whose path contains `'8'`; the .NET installer keeps only the newest hostfxr (9.0.6 here), so every app asserted at startup. Recipe patched to accept any major version ≥ 8
  - Environment: `TEMP` / `TMP` pointing into `AppData` is not writable for the build here; builds run with `TEMP` under `CodeRepository`
- [x] Package strategy (decision 5): keep **`SnowLeopardEngine/xmake-repo`**, rebuilt from the owner's `xmake-repo@backup` plus the SLE recipes
- [x] Upgrade dependencies: fmt → 12.1, entt → 3.16.0, imgui → 1.92.5-docking
  - entt ≥ 3.13 replaced `snapshot::entities().component<T...>()` with one `get<T>()` per storage, and removed `registry::each`. Scene save / load is covered by the new `SceneSerialization` engine test
  - imgui 1.92 — **correction**: the first attempt never took effect, because `3rdParty/ImGuizmo/xmake.lua` also required `imgui v1.90.1-docking` and that require won. The editor really moved to 1.92.5 with step 2.0. It needed the `TabMinWidthForCloseButton` / `ImGuiCol_Tab*` / `NavHighlight` / `AllowItemOverlap` renames, replacements for the removed `GetWindowContentRegionMin/Max`, and two missing end calls (`InspectorPanel` had no `ImGui::End()`, `ResourcePanel` no `EndColumns()`), which 1.92 reports every frame and recovers from with a broken layout. The shipped `imgui.ini` docked into a dock-space ID that 1.92 no longer produces, so the default layout is now built in code (`DockBuilder`) and no ini is copied over the user's own
  - Warnings are still to be cleaned up
- [~] Repair CI: workflows run on `main` / `dev` / `dev-next`, build Debug and Release on Windows and Linux with xmake 3.1.2 and a package / build cache, then run `JoltPhysics_Test` (engine tests need a GL window, which hosted runners lack). The macOS workflow is removed (decision 4). **Not run on GitHub yet**: green only once pushed, and Linux has not been built since the revival
- [x] Run every example and GameDemo; record what works as the regression baseline (see below)

**Regression baseline (2026-11-06, Windows 11, Debug, GL 4.5).** Each app was started, left running for 12 s, then closed through its window.

| App | Starts and runs | Notes |
|---|---|---|
| Application, AudioSystem, InputSystem, LifeTimeComponents | yes | clean shutdown |
| NativeScripting | yes | quits itself by design (`DesktopApp::Quit()` in the script), clean shutdown |
| TickOneFrame | yes | exits after one frame by design |
| PhysicsSystem | yes | both spheres fall and roll on the height field (checked in frame captures). The example had a stray `CharacterController` on the blue sphere, which pinned it, and spawned spheres inside the random terrain; both fixed, camera raised |
| AnimationSystem, GUISystem, OverallShowcase, RenderingSystem-Overview / -PBR / -Instancing | yes | clean shutdown since the ownership fix; before it, Debug exit stopped on the ozz allocator assert (known issue 11) |
| — completeness audit | | AnimationSystem showed a **T-pose**: it never set an entry clip, so the animator never sampled (now the first registered clip is the entry clip, and the example loads both clips and switches with keys 1 / 2). GUISystem placed its UI at fixed 1024 × 768 coordinates, so at fullscreen it sat small in a corner (now laid out from the window centre; its leaked `RenderContext` is owned). OverallShowcase started with the camera inside a dancer (grid centred, camera moved back). The Overview / PBR point lights are commented out in the examples, not missing in the renderer. UI is not scaled with the display, so GUISystem / GameDemo widgets are small at 4K |
| `AnimationPlayback` (engine test) | 9 / 9 pass | Vampire with Dancing + Walking: entry clip defaults to the first clip, mesh leaves the bind pose and keeps moving, bool-parameter transition Dance → Walk → Dance, and the blend starts from the current pose (it used to restart the source clip). Against the old animator 6 checks fail |
| GameDemo | yes | menu only (no input during the run), so the character controller was not exercised; clean shutdown |
| `JoltPhysics_Test` | passes | sphere rests at y = 0.480 on a box floor |
| `PhysicsScenarios` (engine test) | 21 / 21 pass | steps the engine's fixed tick deterministically and checks: sphere / box / capsule / convex-mesh resting, static bodies, triggers, `OnColliderEnter` once per touch, character walking + GameDemo grounding ray, overlaps (bodies and characters), `AddForce`, runtime spawn (incl. mesh + material) and destroy, RigidBody under a CharacterController, height-field orientation against the mesh |
| `SceneSerialization` (engine test) | 23 / 23 pass | scene → JSON → scene: entity count, UUIDs, transforms, physics / mesh / material / camera / light values, a destroyed entity, UUID lookup and new entities after load |
| `InputEvents` (engine test) | 22 / 22 pass | synthetic SDL events → InputSystem: key mapping, down / held / up edges, OS repeat, mouse buttons, wheel, absolute and relative motion, close request |
| SnowLeopardEditor | yes | `xmake run SnowLeopardEditor` (the target's default `--project` is the repo's `ExampleProject.dzproj`): Viewport / Inspector / Resources / Console docked, imgui 1.92.5 on SDL3, no imgui errors. The Viewport shows the default scene (floor, sphere, cube, skybox) at the panel's size; clean exit (the panel list used to keep the editing scene alive past engine shutdown, which tripped ozz's leak assert). Fixed: a relative project path or a bare `argv[0]` made the asset / resource folders empty and the editor exited on `directory_iterator("")` |

**Visual check.** `SLE_CAPTURE_FRAME=<n>` (and optionally `SLE_CAPTURE_PATH`) writes the n-th presented frame to a PNG (`GraphicsContext::CaptureBackBuffer`, GL backend). Every example except TickOneFrame / NativeScripting (they exit at once) was captured and inspected after the rendering fixes. Bodies rest ~2 cm into the ground (Jolt's default penetration slop).

Not covered yet: GameDemo gameplay (it needs mouse input on the menu), editor interaction, and a pixel comparison against `main` (GL reference shots for Phase 2 should be taken from the fixed renderer, not from `main`).

**Fixed alongside:** entities created at runtime (`TriggerEntityCreate`) never got their primitive mesh or material, because only `LogicScene::OnLoad` loaded them; the renderer then dereferenced null. Resource loading is now per entity (`LoadEntityRenderResources`) and runs for spawned entities too, and the renderer skips entities without a mesh or material.

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
  - [x] Windows / MSVC build of everything (Phase 0), and every example starts and runs
  - [x] Visual check of every example via frame capture (after the lighting fixes; no pixel comparison against `main`, whose output was wrong)
- Still GL-bound outside the backend, by design until Phase 2: GLFW context creation in `GLFWWindow`, and the editor's `imgui_impl_opengl3`
- [x] Follow-up: fix the tone mapping / gamma order (known issue 9) in the GL renderer before GL reference screenshots are taken for Phase 2

> Value: whatever shape VRI's API takes, the problem is confined to one folder instead of 16 files.

### Phase 2 — OpenGL → VRI (3–5 weeks)

**Reference material**
- VRI is **public**: `zzxzzk115/VRI` v0.1.17, a C API (`<vri/vri.h>`, `VriCoreInterface` / `VriSwapChainInterface` function tables). libvultra enables only the Vulkan backend and ships a patched `vri-vultra` package (swapchain extent forwarding, cube-array feature, calibrated timestamps).
- libvultra's integration layer is thin: `Device` (VriDevice, queue, tables, pipeline cache), `Frame` (**one frame in flight**; `submitAndWait` is the resource lifetime boundary), `Texture` / `Buffer` (whole-resource state tracking + `transition()`).
- Render graph: libvultra has its **own RenderGraph** (explicit import / transient / history / export with automatic VRI barriers) and intentionally avoids the external `fg` library.
- Shaders: Slang → SPIR-V with reflection, cooking and hot reload.
- libvultra's built-in renderer (deferred/forward, OpenPBR subset, IBL, CSM, tone mapping, meshlets) overlaps heavily with SLE's pipeline.

**Key constraint**: SLE is public and libvultra is private. `includes("external/libvultra")` would make SLE unbuildable for everyone else. Default approach: **SLE depends only on public VRI and ports the code it needs from libvultra** (same author, MIT).

- [x] **2.0 GLFW → SDL3** (decision 9) — done:
  - `Platform/SDL3/SDL3Window` replaces `GLFWWindow`; SDL is private to that file (forward declarations in the header). The `Window` interface gained `SetVSync`, the GL loader, `GetContentScale`, `GetGraphicsContext` and native-event listeners, so `GraphicsContext` and the editor no longer call windowing APIs directly
  - Keys are mapped from SDL scancodes (positional, like the GLFW values `KeyCode` keeps); OS key repeats are ignored as before; mouse coordinates are framebuffer pixels. A hidden cursor uses SDL relative mode and the engine integrates the deltas into an unbounded position, which is what GLFW's disabled cursor gave the camera scripts
  - Gamepads: `GamepadButton` / `GamepadAxis` and `InputSystem::GetGamepadButton{,Down,Up}` / `GetGamepadAxis` for the first connected pad (not tested with a device)
  - Windowed sizes are multiplied by the display scale where SDL coordinates are pixels (Windows, X11), matching the old `GLFW_SCALE_TO_MONITOR`
  - SDL3 is a **shared** package used by the engine DLL and the editor's `imgui_impl_sdl3` (two static copies would not share state); `copy_assets` copies the DLL / `.so` next to every executable, with an `$ORIGIN` rpath on Linux
  - Verified on Windows: the new `InputEvents` engine test (22 checks), the other engine tests, frame captures of every example, the GameDemo menu and the editor. Linux / SDL3 is only covered by CI, which has not run yet
  - Original scope: replace `GLFWWindow` and the GLFW input path with SDL3 (window, keyboard / mouse, **gamepad**, high-DPI, clipboard), behind the existing `Window` / `InputSystem` interfaces. Doing it first means the VRI swapchain is created once, from an SDL3 window, instead of being written for GLFW and then redone. The editor switches to `imgui_impl_sdl3`
**Decision (2026-11-06): OpenGL is dropped outright** — no GL / VRI dual backend and no GL-shaped compatibility layer. The GL-era `RenderContext` (immediate binds, uniforms by name, per-pass FBOs) is not ported; the renderer is rebuilt in VRI's model: explicit device and command buffers, pipelines with descriptor layouts, a render graph that owns barriers, Slang shaders. Until 2.4 lands, apps present only a clear colour (the editor GUI works from 2.1).

**Target layout** (`Function/Rendering/`):
- `RHI/` — thin VRI ownership, ported from libvultra: `Device` (VriDevice + core / swapchain tables + graphics queue + pipeline cache, GPU selection), `Frame` (one frame in flight; `SubmitAndWait` is the resource lifetime boundary, as in libvultra), `Swapchain` (from the SDL3 window via `vriWindowHandleFromSDL3`; resize / out-of-date handling; VSync = present mode), `Texture` / `Buffer` (whole-resource state + `Transition`), upload and readback helpers. VRI stays the public API; these classes only own handles.
- `RenderGraph/` (2.2), `Shader/` (2.3, DzShader v3 → Slang → SPIR-V), `Renderer/` (2.4, the passes).
- `RenderSystem` owns device / swapchain / frame and runs: overlays upload → acquire → scene renderer → overlays draw (editor ImGui) → present; `SLE_CAPTURE_FRAME` reads the swapchain image back.
- Editor ImGui: `imgui_impl_sdl3` for the platform, VRI's built-in ImGui renderer (`vri_ext_imgui`, ImGui 1.92 dynamic textures) instead of `imgui_impl_opengl3`.

- [x] **2.1 VRI core, GL removed** — done:
  - VRI is `vri-vultra v0.2.0` from an in-tree recipe (`xmake/packages/v/vri-vultra`, registered as the `snow-leopard-engine-local` repository): VRI commit d0c78964 plus libvultra's desktop patches. Plain `vri v0.1.19` crashed in device creation on this machine (Intel + NVIDIA ICDs: the time-domain query went through another ICD's trampoline, `calibrated_timestamps.patch`) and its validation layer does not forward `GetSwapChainExtent` (null in Debug, `validation_extent.patch`). The patches are byte-pinned (`.gitattributes`: `-text`). To publish to `SnowLeopardEngine/xmake-repo` later
  - `RHI/Device` (GPU selection: discrete > integrated > other, then VRAM; Y-up clip space like the engine's GL-style cameras; validation on in Debug, `SLE_VULKAN_VALIDATION=0/1` overrides), `Frame`, `Swapchain` (BGRA8_UNORM with RGBA8 fallback, resize / out-of-date / minimized), `Texture` / `Buffer` with state tracking, blocking upload helpers, aligned-row readback. The RTX 4090 is selected here; validation reports nothing
  - `RenderSystem` clears and presents; overlays (`RenderOverlay`: upload before acquire, copy outside, draw inside the swapchain pass). `SLE_CAPTURE_FRAME` reads the swapchain image back
  - Editor: `imgui_impl_sdl3` (`InitForOther`) + `ImGuiRenderer` on `vri_ext_imgui` with ImGui 1.92 dynamic textures. Multi-viewport is off until detached windows get their own swapchains
  - UI textures (`IO::Load(path)`) are uploaded `RHI::Texture`s; `Material` is the `.dzmaterial` definition only; mesh data is CPU-only (`MeshData` lost its GL buffers)
  - Removed with GL: `3rdParty/glad`, `fg`, `shaderc`, `spirv-cross`, the `fg_Test` and the GL-based `FreeType_Test` library tests, GPU Tracy zones (back in 2.6)
  - **Temporary regression**: apps show only the clear colour (no scene, UI images or text) and the editor Viewport is empty until 2.2 – 2.5. Engine tests (physics, serialization, input, animation) pass unchanged; all 13 apps run and exit cleanly
  - Original scope: `vri` v0.1.19 (Vulkan backend); `RHI/` as above; `SDL3Window` creates no GL context; `RenderSystem` clears and presents through VRI; editor ImGui on VRI; frame capture via readback. Deleted: the OpenGL backend, `glad`, `fg`, `RenderContext`, `GraphicsPipeline`, the GL `Texture` / `Buffer` / vertex-format classes, `WorldRenderer` and all passes, the IBL / cubemap / shadow tools, the GL shader compiler (shaderc / spirv-cross). CPU-side data (meshes, materials as files, components) stays
- [x] **2.2 Render graph** — done: libvultra's `RenderGraph` ported to `Rendering/RenderGraph/RenderGraph` (SLE naming, same behaviour): imported / transient / history resources, passes declare `GraphUsage`s and the graph derives every barrier, culling from exports / side effects / history (content dependencies, so a discarding write drops the previous producer), optional aliasing of identical transients with disjoint lifetimes, `CaptureAfterPass`, `Bind` to rebind an import (the swapchain image), snapshot / resource info for inspection. Not ported: libvultra's data-driven `GraphDefinition` / `PassCatalog` (editor graph files) and the GPU profiler hook (2.6)
  - `RenderSystem` presents through a frame graph: import the swapchain image, Clear → Overlays (editor ImGui) → Present (side effect); built once per extent / overlay set, the acquired image is rebound each frame
  - New `RenderGraph` engine test (20 checks): culling, discard semantics, capture of an intermediate value, aliasing (disjoint shares, overlapping and exports do not, aliased plan is frozen), import + rebind, the four validation errors, all with pixels read back from the GPU
  - Original options: `fg` does **not** handle barriers, while VRI requires explicit synchronization:
  - (a) keep `fg` and call `transition()` manually in each pass — small change, easy to get wrong
  - (b) **port libvultra's RenderGraph** and replace `fg` — barriers and transient reuse come for free. **Recommended**; after Phase 1 the setup/execute structure of each pass carries over
- [x] **2.3 Shaders — DzShader v3 on ANTLR + Slang** — core done (`Function/Rendering/Shader`), ported from libvultra's `.vshader`:
  - Format: libvultra's grammar unchanged, as `.dzshader` (`Grammar/dzshader_{lexer,parser}.g4`). The ANTLR 4.13.2 C++ parser is generated into `Grammar/generated` and committed, so builds need no Java; `Tools/Scripts/generate_dzshader_parser.{ps1,sh}` regenerate it with the hash-pinned official jar. Here the generated files were taken from libvultra and renamed (no Java on this machine)
  - `ShaderAsset`: Properties (Float / Range / Integer / Boolean / Vector / Color / 2D / 2DArray / 3D / Cube / CubeArray, attributes HDR / Normal / SRGB / Linear / HideInInspector / NoScaleOffset / Enum), Variants (Defines, link-time Constants), SubShader (Tags, render states, SLANGINCLUDE, Surface, Pass with Name / Tags / LightMode / Vertex / Fragment / Compute). Passes inherit SubShader states. Errors carry file:line:column; Slang diagnostics point into the `.dzshader` through `#line`
  - Properties become `ParameterBlock<DzMaterialProperties> material` (textures add `<name>Sampler` and, for 2D, `<name>ScaleOffset`); every pass is compiled for every variant
  - `ShaderCompiler`: Slang 2026.11 (`slang-prebuilt`, DLLs copied next to every executable) → SPIR-V 1.5 with reflection (parameter tree, resource bindings, push constants, colour outputs)
  - `ShaderPipeline`: VRI pipeline layout from reflection (set index = register space) and a graphics pipeline from the pass states (Cull, FrontFace, ZWrite, ZTest, Blend, BlendOp, ColorMask, DepthBias; `[property]` states use the property default). `ShaderMaterial`: the `material` constant buffer laid out from reflection, texture properties with builtin white / black / normal defaults, validated `Set`
  - New `DzShader` engine test (33 checks): parser model and errors, compile + reflection, and GPU draws read back (material values, texture builtin, Boolean, variant define, property-driven Cull, push constants)
  - Not yet: Surface shaders, `Requires` / mesh stages, variant `Modules`, Stencil states, non-2D texture properties in materials, a shader disk cache and hot reload. (The JSON DzShader v2 files and GLSL were removed in 2.4)
  - Original scope: replace the JSON DzShader with a ShaderLab-style text format (Properties / SubShader / Pass / render states / HLSL-like Slang program blocks), parsed by an ANTLR grammar and compiled by Slang to SPIR-V with reflection. libvultra's `.vshader` (`vshader_lexer.g4` / `vshader_parser.g4`, `shader_parser.cpp`, `shader_compiler.cpp`, last-valid hot reload) is the reference. Pipeline states declared in the shader (`Cull`, `ZTest`, `ZWrite`, `Blend`) map onto the engine-owned enums from Phase 1. Port the 25 GLSL / DzShader files; Vulkan conventions (Y flip, [0,1] depth) are handled once in the shader library
- [x] **2.4 Scene passes, GPU-driven** — done (`Rendering/Renderer/SceneRenderer`):
  - **GPU-driven** (decided 2026-11-07): every mesh in one vertex / index arena (uploaded once, bounding sphere computed once), the frame's objects in an instance buffer, materials in a material buffer whose maps are indices into a bindless texture table (`VriFeature_Bindless`, partially bound array of 1024). A compute pass frustum-culls the instances per view (camera, 4 shadow cascades, transparents) and writes indexed indirect draws; each geometry pass is one `DrawIndexedIndirectCount`. Instances reach the vertex shader through an identity per-instance vertex stream (`baseInstance` = instance), so no `shaderDrawParameters` / `multiDrawIndirect` is needed (VRI enables neither); transparents are sorted on the CPU and culled in place (zero-instance draws keep the order). Without the count extension the commands are drawn one by one
  - Frame: upload CPU-skinned vertices (in-frame copy into their arena range) → clear counts → cull → 4 cascade shadow maps (libvultra's sphere-fit, texel-snapped cascades; 2048², 200 m, hardware PCF 4 × 4, cascade blending) → G-buffer (albedo + metallic sRGB8, normal + roughness RGBA16F, emissive + AO RGBA16F, depth; world position reconstructed from depth) → deferred lighting + sky in one fullscreen pass (GGX sun + 16 point lights, split-sum IBL with multiple-scattering compensation from equirectangular irradiance / GGX-prefiltered specular / BRDF LUT made at load from the camera's `.hdr`, libvultra's `environment.slang`) → transparent forward (alpha blend) → ACES tone mapping into the target
  - Shaders: `Assets/Shaders/Standard/Standard.dzshader` (GBuffer / Forward / ShadowCaster; its Properties are the `.dzmaterial` vocabulary and validate material files), `Renderer/{Culling,DeferredLighting,ToneMapping,Environment}.dzshader`, `Slang/{Common,Scene,Lighting}.slangh`. The JSON v2 `Standard` / `StandardDeferred` are gone
  - Fixed on the way (GL renderer behaviour): point lights ignored their Constant / Linear / Quadratic attenuation; the sky `.hdr` was uploaded as 8-bit (highlights clipped); textures had no mipmaps; the AO map was sampled but discarded; IBL specular was halved and an extra constant ambient added on top of IBL; Standard's default `ao` was 0 (black IBL); per-frame CPU AABB rebuilds over every vertex of every object; no directional light → nothing rendered; `CameraClearFlags::Color` ignored (now the clear colour replaces the sky). Engine: entities created at runtime never got meshes or materials (resources were loaded in `TriggerEntityCreate`, before any component existed) — scenes now load pending render resources at the start of each tick, and `LoadEntityRenderResources` is idempotent
  - **Offline mode**: `WindowInitInfo::Offline` / `SLE_OFFLINE=1` — hidden window, no swapchain, frames rendered into an offscreen texture (captures read it). GPU engine tests run offline; captures use it
  - New `SceneRendering` engine test (14 checks, offline): sky, lit object, cascaded shadow, GPU cull counts (object behind the camera culled), transparent blend, bindless textured material, point-light attenuation, clear colour. Render graph: new `GraphUsage::IndirectRead`. RHI: mip / format-generic texture upload, mip views, buffer views, region uploads, HDR loading
  - Verified on Windows (RTX 4090): all 13 apps and the editor run offline with zero validation messages; PBR / Showcase / Animation / Physics match the GL captures (same composition; brighter, unclipped sky reflections). All 7 engine tests pass
  - **Grass / foliage** (redesigned, not ported): an `AlphaTest` render queue in the same GPU-driven path. Two-sided cut-out G-buffer and shadow pipelines (`GBufferAlphaTest`, `ShadowCasterAlphaTest`), own culling views (camera + 4 cascades), so grass is lit, receives and casts (cut-out) shadows. Wind is a material property (`windStrength`, `windDirection`, `noiseMap`): bend weighted by blade height (roots stay put), drifting noise gusts, culling sphere grown by the swing. `Next/Grass.dzmaterial` uses it. Identical static meshes are deduplicated in the arena by content hash (Instancing's 10 000 grass entities share one quad), and new meshes are uploaded in one transfer per frame instead of one blocking submit each. `EnableInstancing` is obsolete: everything is instanced
    - GL behaviour replaced: unlit grass blitted over the lit scene, whole blades sliding along the ground, the wind direction's `.xy` used as `.xz`, grass picked by material name, per-batch uniform arrays of 250 matrices
  - **In-game UI** (redesigned): `UIRenderer` batches every image, button and glyph of the frame into one vertex buffer drawn with one indexed draw (bindless textures, premultiplied alpha), in painter's order by `RectTransform` Pos.z, after tone mapping (exact UI colours). Rects rotate correctly about their pivot. Text: `FontAtlas` — FreeType SDF glyphs (48 px base, 8 px spread) packed on demand into one atlas per font file, UTF-8, kerning, `\n`, antialiased at any size; `TextComponent::FontFilePath` and `TextAlignment` are honoured, `FontSize` is the em size in pixels (default 48, Left alignment = the old behaviour; GameDemo sizes ×4 keep their look)
    - GL behaviour replaced: one draw and one vertex upload per glyph, one texture per glyph, glyphs cut at 0.9 coverage (aliased), ASCII only, `SrcColor / OneMinusSrcColor` blending, a pivot translation applied twice, a depth buffer for UI order, font file and alignment ignored, `FontSize` in units of 4 px
  - **Post-processing** (redesigned; `Renderer/PostProcess.dzshader`, switches and parameters in `SceneRendererSettings`, `SetSettings` rebuilds the graph):
    - Ambient occlusion: GTAO (horizon search in 2 screen-space slices × 8 steps per side, analytic cosine-weighted visibility, world-space radius projected per pixel, interleaved-gradient noise) + depth-aware 5 × 5 denoise; multiplies the material AO, so it occludes indirect (IBL) light only
    - Bloom: 6-level downsample / upsample chain (13-tap downsample with Karis-weighted first level, texel-sized tent upsample adding each level's detail), mixed energy-conservingly into the HDR image inside the tone-mapping pass
    - FXAA 3.11 (12 search steps) on the tone-mapped image with luma written by tone mapping, **before** the UI
    - GL behaviour replaced: Crytek-style hemisphere SSAO with a fixed kernel and a tiled 4 × 4 noise texture, blurred with a depth-unaware Gaussian (halos across silhouettes); bloom upsample radius a fixed UV offset (aspect-dependent); FXAA after the UI blit (blurred UI text)
    - Found by the new A/B checks: the lighting read the AO map with `Load(pixel)`, so the 1 × 1 white placeholder (AO off) read 0 outside its texel and removed all image-based light — now sampled by uv
  - The GL-era GLSL and JSON v2 shaders are removed from `Assets/Shaders` (all superseded; in git history). Weighted-blended OIT is replaced by sorted forward transparency
  - `SceneRendering` engine test: 23 checks (+ alpha-test queue, mesh sharing, cut-out grass, UI order, SDF text, AO / bloom / FXAA on-off renders). `SLE_TEST_DUMP=1` writes the A/B frames as PPM
  - Follow-ups (not blocking): Hi-Z occlusion culling, GPU skinning (Animator skins on the CPU), mesh-arena compaction (stale ranges are freed on scene change), UI clipping / masks, auto exposure, TAA; the editor Viewport is 2.5
  - Original pass order: Blit / Final → GBuffer + DeferredLighting → ShadowPrePass / CSM → Skybox / IBL precompute (CubemapConverter, IBLDataGenerator as compute) → SSAO / Bloom (Down/Upsample, GaussianBlur) / FXAA / ToneMapping → WeightedBlended + TransparencyCompose → Grass → InGameGUI (glyph atlas, no `glBindTextureUnit`)
  - Direct references in libvultra: `builtin_renderer.cpp` (GBuffer / DeferredLighting / Skybox), `shadow_cascades.cpp`, `tone_mapping_pass.cpp`, `environment.cpp` (IBL)
- [x] **2.5 ImGui + editor Viewport** — done (the ImGui backend itself landed in 2.1: imgui 1.92.5 docking, `imgui_impl_sdl3` + `vri_ext_imgui` with dynamic textures):
  - `RenderSystem` render-to-texture: `UpdateViewport` (called by the Viewport panel before ImGui records the image) creates / resizes an RGBA8 viewport texture, so the view ImGui records is the one rendered that frame; the frame graph imports it, the scene renderer renders into it at the panel's size (camera aspect follows), and the overlay pass declares it `Sampled`, so the graph puts it in the shader-read layout before ImGui draws. `GetOutputTextureID` is its view (the `ImTextureID` of the editor's ImGui renderer). In-game UI and post-processing apply in the Viewport too
  - `ImGui::Image` no longer flips v (textures are top-down on VRI)
  - Verified offline: the editor's default editing scene renders in the Viewport, zero validation messages; apps (window path) unchanged
  - Not here: object picking (already disabled in the GL version) and gizmo interaction belong to Phase 4 (editor); multi-viewport (detached ImGui windows need their own swapchains) is off
  - Original scope: upgrade the editor to imgui 1.92.x docking with a VRI renderer backend (following libvultra's EditorGui); the Viewport samples a texture exported by the render graph
- [x] **2.6 Cleanup — GPU profiling** — done:
  - `RHI::GpuProfiler` (ported from libvultra's Profiler, VRI timestamp queries via `vri_ext_query`): timestamps at each pass's start, after its barriers and at its end; resolved into a readback buffer and collected after `SubmitAndWait`. `RenderGraph::Execute(cmd, profiler)` times every live pass, so there are no per-call-site GPU macros (`SNOW_LEOPARD_PROFILE_GPU` removed)
  - `RenderSystem` profiles every frame: `GetPassTimings()` (per pass: CPU record time, GPU time, GPU barrier time) and `GetFrameGpuMs()`
  - Tracy: the timings are emitted as GPU zones of a "VRI" GPU context through Tracy's manual GPU API (no Vulkan handles needed), only while a Tracy client is connected, so nothing is queued when no profiler is attached. Not verified with a Tracy client yet (needs the Tracy GUI)
  - `SceneRendering` engine test: 26 checks (+ timestamp support, every live pass profiled, positive pass times within the frame)
  - Kept on purpose: `CameraSystem` still returns OpenGL-convention projections, because the editor's ImGuizmo consumes them; the scene renderer remaps depth to [0, 1] in one place. GL remains only in comments and the editor's file-type icon table
  - Original scope: Tracy GPU zones via VRI timestamps (`glad` and the GL `GraphicsContext` went in 2.1)
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
Started 2026-11-08 (ahead of Phase 3: the editor had fallen behind the old `editor` branch). Redesigned rather than ported:
- `EditorContext` owns what the panels share: the edited scene and its file, play mode (a `LogicScene::Copy` is simulated and dropped on Stop; saving always writes the edited scene), the selection (UUIDs, valid in the copy too) and the editor camera. The old `EditorCameraScript` was an entity of the scene, so it was saved into scenes, copied into play mode and rendered as the "first camera"; the new `EditorCamera` is not an entity and the renderer draws through it with `CameraOverride` while editing (play mode uses the scene's camera). Its movement is per second (was per frame), W / E / R no longer switch gizmos while flying, Alt orbit and middle-mouse pan were added
- Picking (`SceneRendererSettings::EditorOverlay`): an `EntityID` pass rasterizes the camera's culled indirect draws (opaque, alpha-tested with cut-out and wind, transparent, CPU-skinned) into an R32_UINT target with its own depth; `RequestPick` copies one texel to a readback buffer collected after the frame's submit (one frame of latency, no stall). The same buffer drives the selection outline, an overlay pass after FXAA, so occluders cut the outline. Games pay nothing (no pass without the setting). Replaces the GL back-buffer hack
- Scene graph: `LogicScene::GetWorldTransform` (the renderer, gizmos and point lights use world transforms), `SetParent` (keeps the world transform, refuses cycles), `DuplicateEntity` (with children), `GetRootEntities`. Fixed on the way: `DestroyEntity` erased from the child list it iterated; `GetEntityWithCoreUUID` threw on unknown ids; `SceneManager::SetActiveScene(nullptr)` kept the scene; `LogicScene::Copy` reversed entity order; `SceneRenderer::SetSettings` rebuilt the frame graph on every slider move; the Resources panel opened every file of a folder on any double-click and sent a truncated drag payload
- Verified: `SceneRendering` 31 checks (picking, empty pick, camera override, outline), `SceneSerialization` 37 (reparenting, cycles, duplicate, delete subtree, hierarchy save / load, play-copy order); offline editor captures (`SLE_EDITOR_SELECT=<name>`, `SLE_EDITOR_PLAY=1`) show the layout, gizmo, outline on static and skinned meshes, and physics running in play mode. Not verified: mouse interaction itself (clicks, drags, dialogs) — needs a person at the editor
- [x] Menu bar / toolbar (new / open / save scene; Play / Pause / Stop) — project save stays on exit
- [x] #72 Hierarchy (drag-to-parent, context menu, multi-select, rename, filter)
- [ ] SMeta generator → #73 auto-drawn Inspector (the manual Inspector now covers transform, mesh, material, camera, lights, rigid body, colliders, animator, add / remove)
- [ ] #131 Material Inspector, Settings and Preview panels (#136 RenderSettings done)
- [ ] Asset pipeline: a real AssetManager; clear the 7 TODOs in `LogicScene`; runtime (ozz) models referenced from scenes (the default scene's character is not saved); drag-and-drop of material / model paths into the Inspector works
- [x] Gizmos and picking via an ID buffer
- [ ] Undo / redo, multi-viewport (detached panels need per-window swapchains), camera / light icons in the Viewport

### Phase 5 — Gameplay Systems (order as needed)
- [ ] Animation: #70 BlendJob, #134 `.dzanimcon`, state-machine view in the editor (#69 blackboard done)
- [ ] Physics: #133 `.dzphymat` (damping / CCD are done in the Jolt port), scene-unload hook (today the world is rebuilt when the next scene is cooked), Jolt debug renderer in the editor
- [ ] **In-game GUI and text**: fix text rendering (glyph atlas, correct metrics / baseline / kerning, UTF-8), then replace the naive Canvas / Image / Text / Button system with a real retained UI (layout, anchors, styling, focus and input routing). Evaluate RmlUi against a home-grown widget set before writing code
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
   - B. Depend on libvultra as a subproject (`includes("external/libvultra")`) so SLE is only the game layer (Jolt / ozz / miniaudio / editor / gameplay). Least code, but requires libvultra to be public, and libvultra's architecture rules (Godot-style node tree) differ from SLE's EnTT component model
   - C. Archive SLE and use its physics / animation / audio work as input to libvultra's "optional extensions" roadmap item
2. **Frame graph**: **port libvultra's RenderGraph** vs. keep `fg`.
3. **Shader language**: ~~Vulkan GLSL first, Slang later~~ → **decided: Slang, with DzShader v3 as a ShaderLab-like syntax parsed by ANTLR** (following libvultra's `.vshader`).
4. **Platforms**: ~~drop macOS temporarily~~ — **revised 2026-11-08: macOS stays a target.** VRI runs there through MoltenVK and has a native Metal backend (the `vri` recipe's `metal` config); the other dependencies support macOS. Still to do: a macOS CI job and a first build / run on a Mac.
5. **Packages**: ~~in-repo `external/packages` recipes~~ → **decided: keep `SnowLeopardEngine/xmake-repo`, synced from the owner's `xmake-repo` (`backup` branch: recent upstream + the Vultra packages VRI / vshadersystem / slang-prebuilt / antlr-cpp that Phase 2 needs) plus the SLE recipes on top.** The old fork was a 2024 copy of the whole upstream and shadowed newer official recipes.
6. **C++ / runtime**: whether to follow libvultra to C++23 + static MSVC runtime (MT). Jolt, ozz, etc. must switch together to avoid LNK2038. Undecided; stays C++20 / MD until Phase 2.
7. **Branching**: ~~long-lived `revival` branch~~ → **decided: long-lived `dev-next` branch, one PR per phase. Only `main`, `dev` and `dev-next` remain**; the 20 old feature branches are deleted (every tip is kept in a git bundle, `old-branches-2026-11-06.bundle`, outside the repo).
8. **Physics**: ~~PhysX (`sphysx-cross`)~~ → **decided: Jolt Physics** (official xmake recipe, no custom package, no GPU DLLs, MIT). Done in Phase 0.
9. **Windowing / input**: ~~GLFW~~ → **decided: SDL3**, as step 2.0 before the VRI swapchain.
10. **GameDemo**: **rewrite rather than port** (Phase 6). It only has to keep building until then.

## 5. Risks
- ~~Custom packages and old pinned versions may stall the first build~~ — happened (see Phase 0); resolved. Remaining custom recipes: `scoral` (until Phase 3), `fg` (until 2.2), `fastnoise2`.
- Jolt port: the character controller kept PhysX semantics (capsule-center position, displacement-based `Move`), but feel and edge cases (steps, slopes, pushing) can differ. GameDemo gameplay was not exercised yet; the rewritten demo (decision 10) is where it gets tuned.
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
| 2026-11-06 | 0 | Package repo rebuilt from the owner's `xmake-repo@backup` + SLE recipes (`ozz-animation` MDd and `scoral` hostfxr fixes) |
| 2026-11-06 | 0 | PhysX → Jolt Physics 5.5.0; `JoltPhysics_Test` replaces `PhysX_Test` |
| 2026-11-06 | 0 | fmt unpinned (12.1, shared with spdlog), `/utf-8` on MSVC, enum log formatter |
| 2026-11-06 | 0 | First Windows / MSVC build of everything; all 13 examples and GameDemo start and run (baseline above) |
| 2026-11-06 | — | Decisions: SDL3 replaces GLFW; GameDemo to be rewritten; work moves to `dev-next`, old branches removed. Known issues added: tone mapping / gamma order, text rendering, naive GUI, ozz Debug exit assert |
| 2026-11-06 | 0 | Mesh / material / texture ownership moved to RAII (`Ref` / `Scope`), scenes destroyed first at shutdown; all 14 apps now shut down cleanly in Debug |
| 2026-11-06 | 0 / 1 | Rendering: tone mapping / gamma order, exposure, grass decode, shadowed IBL, leaking material uniforms. Frame capture (`SLE_CAPTURE_FRAME`) and a visual check of every example |
| 2026-11-06 | 0 | Physics: `PhysicsScenarios` engine test (21 checks); RigidBody under a CharacterController is ignored with a warning; runtime-spawned entities get meshes / materials; PhysicsSystem example fixed |
| 2026-11-06 | 0 | entt 3.16.0, imgui 1.92.5-docking; `SceneSerialization` engine test; editor starts with a relative project path; `copy_assets` no longer flattens `Assets/` on a target's first build; `$(buildir)` → `$(builddir)` |
| 2026-11-06 | 0 | CI: Windows + Linux, Debug + Release, cached packages, Jolt test; macOS workflow removed |
| 2026-11-06 | 2.0 | GLFW → SDL3 3.4.0 (window, GL context, input, gamepad API, editor backend); the editor really on imgui 1.92.5 (ImGuizmo's pin removed, API renames, missing end calls, default layout in code); `InputEvents` engine test |
| 2026-11-06 | 0 | Example completeness audit: AnimationSystem T-pose (animator entry clip, blend timing), GUISystem layout at fullscreen, OverallShowcase camera; `AnimationPlayback` engine test; editor diagnosed as unable to edit (no scene, camera, render target or picking) |
| 2026-11-06 | 0 | Editor Viewport renders: render-to-texture in `WorldRenderer` / `FinalPass`, camera aspect from the render viewport instead of the window, default editing scene with editor camera, GUI clears the window behind it, panels released before engine shutdown |
| 2026-11-06 | 2.1 | OpenGL removed; VRI 0.2.0 (in-tree `vri-vultra` recipe with libvultra's patches): device / frame / swapchain / resources, clear + present, frame capture by readback, editor ImGui on `vri_ext_imgui`. Scene rendering is gone until 2.4 |
| 2026-11-06 | 2.2 | libvultra's RenderGraph ported (barriers, culling, aliasing, history, capture, rebind); the frame is a graph (clear, overlays, present); `RenderGraph` engine test (20 checks) |
| 2026-11-06 | 2.3 | DzShader v3: libvultra's ANTLR grammar as `.dzshader`, parser + validation, Slang 2026.11 → SPIR-V with reflection, VRI pipelines from pass states, `material` parameter blocks; `DzShader` engine test (33 checks) |
| 2026-11-07 | 2.4 | GPU-driven scene renderer: mesh arena, instance / material buffers, bindless textures, compute frustum culling + indirect-count draws; CSM shadows, deferred PBR + IBL + sky, transparent forward, ACES; offline mode (hidden window, no swapchain); runtime entities get their resources; `SceneRendering` engine test (14 checks). Grass, UI, bloom / SSAO / FXAA still to port |
| 2026-11-07 | 2.4 | Foliage as an `AlphaTest` queue (cut-out, lit, shadowed, wind, GPU-culled; mesh dedupe + batched uploads); in-game UI as one batch with an SDF font atlas; `SceneRendering` now 20 checks; all apps run offline with zero validation messages |
| 2026-11-07 | 2.4 | Post-processing: GTAO + depth-aware denoise, bloom chain, FXAA 3.11 before the UI; AO placeholder bug found by A/B checks; GL-era shaders removed; `SceneRendering` 23 checks. 2.4 done |
| 2026-11-07 | 2.5 | Editor Viewport on VRI: render-to-texture viewport texture (resized from the panel), scene passes into it, overlays sample it through the graph; ImGui uv unflipped |
| 2026-11-08 | 2.6 | GPU profiling: `RHI::GpuProfiler` (VRI timestamps) times every render-graph pass; `RenderSystem::GetPassTimings`; Tracy GPU zones while a client is connected; `SceneRendering` 26 checks. Phase 2 done |
| 2026-11-08 | 4 | Editor past the old `editor` branch: `EditorContext`, editor camera outside the scene (`CameraOverride`), GPU entity-id picking + selection outline, gizmos on world transforms, Hierarchy, menu bar / shortcuts / scene files, Inspector add / remove, Render Settings panel; scene-graph world transforms and fixes; `SceneRendering` 31, `SceneSerialization` 37 checks |
