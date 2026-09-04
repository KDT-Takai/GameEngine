# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Git: never commit without being asked

This repo is the user's personal DirectX12 study project. They want to review every diff and write every
commit themselves. Never run `git commit` (or `git push`) here on your own initiative — even after finishing
a task that would normally end with a commit elsewhere. Stage/inspect changes if useful, but leave the actual
commit to the user unless they explicitly ask you to commit.

## Detailed docs (read these for anything not covered here)

- `Debugger/docs/PROJECT_MAP.md` — directory map, per-module purpose, current implementation status, and a running list of gotchas discovered while working in this repo.
- `Debugger/docs/CONVENTIONS.md` — actual naming/file-structure conventions observed in this codebase (not the generic template in `docs/NAMING.md`, which predates this project and doesn't match its patterns).

This file covers only what those two don't: build/run commands and the cross-file architecture.

## Build

Visual Studio 2022 (v143 toolset) solution, MSVC only. No CI, no test suite, no linter configured — don't invent commands for these.

```powershell
& "F:\visualstudioIDE\MSBuild\Current\Bin\MSBuild.exe" DirectX12.sln /p:Configuration=Debug /p:Platform=x64 /m
```

(VS2022のインストール先はマシンによって異なる。`vswhere -all -property installationPath`で確認できる。)

- Add `/t:App`, `/t:Engine`, or `/t:Debugger` to build a single project.
- Configurations: `Debug` / `Release`. Platforms: `x64` (the only one actually verified) / `Win32` (present but unexercised).
- `Debugger` only builds under `Debug` — the `.sln` has no `Release|x64.Build.0` entry for it, and `App.vcxproj` only references it under `Condition="'$(Configuration)'=='Debug'"`. This is intentional: `Debugger` holds tooling that must never ship (see Architecture below).
- Build output goes to `x64\<Configuration>\` at the repo root, not per-project folders.

## Run

Run `App.exe` with its working directory set to `App\` (Visual Studio's F5/Ctrl+F5 does this automatically). Asset and log paths are relative to that directory (e.g. `../App/Assets/`, `../Debugger/Logs/latest.log`) — running the exe from anywhere else breaks asset loading.

After a run, read `Debugger/Logs/latest.log` (Debug builds only, overwritten every run) instead of asking the user to paste console output.

## Architecture

Three-project solution: `App` (exe; `main.cpp` just calls into `Framework`) → `Engine` (static lib; everything a Release build needs) → `Debugger` (static lib; Debug-only tooling). `Engine::System::Framework` (`Engine/Source/Framework/Framework.cpp`) is the single orchestrator — it owns every subsystem singleton and drives `Initialize` → `Run` (main loop) → `Finalize`. Read it first when tracing how systems connect to each other.

**Rendering** is DX12 (`Graphics/DX12/*`: Device / Renderer / RendererContext / DescriptorHeapManager), with two independent draw paths both driven from `Framework::Run()`'s per-frame loop: `SpriteRenderSystem` + `SpriteRenderer` for 2D, `ModelRenderSystem` + `ModelRenderer` for 3D models loaded from this engine's own `.mdl` binary format. Each `~RenderSystem` walks an `entt::registry` view and calls the matching `~Renderer::Draw()` — this Component → System → Renderer chain is the pattern to follow for a new drawable type.

**ECS** is `entt`, wrapped by the `Engine::System::ECS::Registry` singleton. Components are plain data (`TransformComponent`, `SpriteComponent`, `ModelComponent`, `CameraComponent`); systems are the `~System` / `~RenderSystem` classes that operate on them.

**Assets** flow through the `Engine::System::Assets::AssetManager` singleton: extension → loader callbacks are registered in `Framework::RegisterAssetLoaders()`, then `LoadDirectoryOrdered()` walks `App/Assets/` textures-before-models, so model materials can resolve texture IDs that are already registered. `AssetManager::GetTextureID()` matches by filename first, then falls back to "the only texture in the same directory as the model" when a converted model's embedded texture reference doesn't match any real filename on disk (common with FBX exported from Unity or similar, carried through from the source FBX at conversion time) — see `Debugger/docs/PROJECT_MAP.md` for the fix history behind this.

**Debug/Release separation**: anything that must not ship (currently `EntityInspector`, `AssetBrowser`, file logging via `DevLogSink`) lives in the `Debugger` project and is only called from `Engine` inside `#ifdef _DEBUG` (see `Framework.hpp`/`.cpp`, `Utility/Logger/Log.hpp`). To add new debug-only code: put its `.cpp`/`.hpp` in `Debugger.vcxproj`'s `ClCompile`/`ClInclude` (existing convention keeps the files physically under `Engine\Source\...` and references them by relative path from `Debugger.vcxproj`), then guard every call site in `Engine` with `#ifdef _DEBUG`.

**Singletons** all use one pattern: `class X : public Singleton<X> { DECLARE_SINGLETON(X) ... }`, lifecycle `X::Create(args...)` / `X::GetInstance()` / `X::Delete()` (`Utility/Singleton/Singleton.hpp`).

## Critical: source file encoding

Most files under `Engine/Source` are saved as **Shift-JIS (CP932)**, not UTF-8. Editing them with a UTF-8-assuming method silently corrupts Japanese comments into unrecoverable `<27>` replacement characters — this has already happened once in this repo's history.

Before editing a file with Japanese content in it:
1. Check whether it's actually CP932 — most `.cpp`/`.hpp` under `Engine/Source` are; `.vcxproj` / `.sln` / `.filters` / `.md` files are UTF-8 and safe to edit normally.
2. If it's CP932, do a byte-safe replace instead of a normal text edit: read the file via `[System.Text.Encoding]::GetEncoding(932)`, do a literal (non-regex, or single-line-only regex) string replacement, and write it back with the same encoding. Never run a multi-line/DOTALL regex across a CP932 file — an overly broad match has previously deleted entire functions by accident.

## Tools

`Tools/` — built CLI tool (`ModelConverter.exe` + `assimp-vc143-mt.dll`) that converts `.fbx` to this engine's own binary model format `.mdl`. Source lives in the separate [ModelConverter](https://github.com/ShaF-u/ModelConverter) repo; only build artifacts are checked in here. Runtime side: `Engine/Source/Graphics/Model/BinaryModelLoader/` reads `.mdl`, registered in `Framework::RegisterAssetLoaders()` — it is the **only** model loader in the engine now. The old Assimp-based `ModelLoader` (direct `.fbx`/`.obj` runtime loading) was removed 2026-09-05 once every asset had a converted `.mdl`; Assimp is no longer a runtime dependency anywhere in `App`/`Engine`/`Debugger`, only inside the standalone ModelConverter tool. Any new model must be run through `ModelConverter.exe` before the engine can load it — there is no more direct-FBX fallback. `App/Assets/Models/Kipfel.mdl` is a converted-and-committed test asset; see `Tools/README.md` for regeneration steps.

## Other things worth knowing before touching them

- HLSL constant buffers need `#pragma pack_matrix(row_major)` to match DirectXMath's row-vector convention. `Model.hlsl` has it; `Sprite.hlsl` and `Triangle.hlsl` currently don't (latent bug, not yet hit in practice).
- These two FBX-parsing gotchas no longer apply to this repo directly (the Assimp-based runtime `ModelLoader` was removed) but still apply to the conversion tool's Assimp material processing — see `ProcessMaterial` in the separate [ModelConverter](https://github.com/ShaF-u/ModelConverter) repo's `Source/main.cpp`, which this logic was carried over from:
  - It checks `aiTextureType_DIFFUSE` → `BASE_COLOR` → `EMISSIVE` in that order and reads whichever color property matches — not always `AI_MATKEY_COLOR_DIFFUSE`. FBX exporters (Unity's in particular) don't always put the color map/tint in the legacy Diffuse slot.
  - Don't pass texture-path strings embedded in FBX materials through `std::filesystem::path`. They can contain non-ASCII bytes from the original author's machine, and the narrow→wide conversion throws `std::system_error` under the default locale. Split the string manually instead.
