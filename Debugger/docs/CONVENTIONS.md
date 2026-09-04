# ファイル構成・命名規則

このファイルは、実際のコードから観測した「このプロジェクト(C++ / DirectX12)で
実際に使われているファイル構成・命名規則」をまとめたものです。

`docs/NAMING.md` は元々 Web/DBバックエンド向けの汎用テンプレート(DTO/Controller/
Repository、API endpoint、DBテーブル命名など)で、このC++コードベースの実態とは
一致しない部分があります。**このプロジェクトで実際に守るべきルールはこのファイルを
優先してください。** ブランチ命名など汎用的に使える部分は `docs/NAMING.md` の該当箇所
を参照します(このファイル末尾にも要約を載せています)。

---

## 1. ディレクトリ・ファイル構成

### 基本パターン: 1クラス1フォルダ

```
Source/<Category>/<SubCategory>/<ClassName>/<ClassName>.hpp
Source/<Category>/<SubCategory>/<ClassName>/<ClassName>.cpp
```

**例:**
```
Source/Graphics/Model/ModelRenderer/ModelRenderer.hpp
Source/Graphics/Model/ModelRenderer/ModelRenderer.cpp
Source/System/Assets/AssetManager/AssetManager.hpp
Source/System/Assets/AssetManager/AssetManager.cpp
```

ヘッダー専用(データ構造体・小さいコンポーネントなど)の場合は `.hpp` のみでも良い
(例: `TransformComponent.hpp`, `TextureID.hpp`)。

### カテゴリの意味

| カテゴリ | 意味 | 例 |
|---|---|---|
| `Graphics` | 描画・GPUに関わるもの | DX12, Model, Sprite, Texture, Shader, ConstantBuffer |
| `System` | OS・エンジンのサブシステム | Window, Input, Camera, ECS, Assets, ImGui, Screen |
| `Utility` | 横断的な補助機構 | Logger, Singleton, EngineContext |
| `Framework` | 初期化・メインループの統括 | Framework.hpp/.cpp のみ(1ファイル) |

### デバッグ専用コードの置き場所

Release版に含めたくないコード(ImGuiベースのツール類など)は `Debugger/` プロジェクト
配下に置く。物理ファイルは `Engine\Source\...` に残したまま `Debugger.vcxproj` から
相対パスで参照する形と、`Debugger/Source/...` に新規で置く形の両方がある。
詳細は [`PROJECT_MAP.md`](./PROJECT_MAP.md) の「Debugger方式について」を参照。

---

## 2. 命名規則

### namespace

`Engine::<Category>::<SubCategory>` の形。カテゴリはディレクトリ構成と一致させる。

```cpp
Engine::Graphics
Engine::Graphics::Model
Engine::System
Engine::System::Assets
Engine::System::Debug        // EntityInspector
Engine::Utility
Engine::Debugger::Logging    // Debugger専用コード
```

### クラス・構造体名: PascalCase

役割を表す接尾辞(サフィックス)にパターンがある:

| サフィックス | 役割 | 例 |
|---|---|---|
| `~Manager` | ライフサイクル管理・ID→実体のキャッシュ | `TextureManager`, `AssetManager`, `ModelManager` |
| `~Loader` | 外部ファイルの読み込み専任 | `ModelLoader`, `RuntimeShaderLoader` |
| `~Renderer` | 実際の描画コマンド発行 | `SpriteRenderer`, `ModelRenderer` |
| `~RenderSystem` | ECSの `view<>` を回して `~Renderer` を呼ぶ | `SpriteRenderSystem`, `ModelRenderSystem` |
| `~Component` | ECSコンポーネント。データのみ、ロジックを持たない | `TransformComponent`, `SpriteComponent`, `ModelComponent` |
| `~System` | 状態を持つロジック(RenderSystemより広義) | `CameraSystem` |
| `~Context` | スレッド単位などで分離される実行コンテキスト | `DX12RendererContext` |
| `~ID` | 実体を指す軽量ハンドル(`uint64_t`) | `TextureID`, `ModelID` |

### メンバ関数: PascalCase、動詞始まり

ライフサイクルを持つクラスは以下の名前で統一する:

```cpp
Initialize(...)   // 生成・初期化。bool を返し失敗を伝える
Update(...)       // 毎フレームの更新
Draw(...)         // 描画コマンド発行
Finalize()        // 明示的な後始末
```

シングルトン(`Singleton<T>` 継承 + `DECLARE_SINGLETON(ClassName)` マクロ)は
以下のライフサイクルAPIで統一されている(`Utility/Singleton/Singleton.hpp`):

```cpp
T::Create(args...)   // 生成(コンストラクタ引数を転送)
T::GetInstance()     // 参照取得(未生成ならassert)
T::GetPtr()          // 生ポインタ取得(未生成ならnullptr)
T::IsCreated()        // 生成済みか
T::Delete()          // 破棄
```

### メンバ変数: camelCase、接頭辞なし

`m_` や `s_` のような接頭辞は使わない。

```cpp
std::unique_ptr<Engine::Graphics::TextureManager> textureManager;
Engine::Graphics::SpriteRenderSystem              spriteRenderSystem;
Engine::System::Camera::CameraSystem              cameraSystem;
```

### 定数・マクロ: UPPER_SNAKE_CASE

```cpp
LOG_TRACE(...) / LOG_DEBUG(...) / LOG_INFO(...) / LOG_WARN(...) / LOG_ERROR(...) / LOG_CRITICAL(...)
LOG_HRESULT(msg, hr)
DECLARE_SINGLETON(ClassName)
```

### ID型: `using` + `Invalid~` センチネル定数のペア

```cpp
using TextureID = uint64_t;
static constexpr TextureID InvalidTextureID = 0;
```

新しいID型を追加するときはこのペアを踏襲する(`ModelID` も同型)。

### enum: `enum class`、値もPascalCase

```cpp
enum class ProjectionType
{
    Orthographic,
    Perspective
};
```

### コメント

コード中のコメントは日本語で実装意図を書く。**ただし多くのソースファイルは
Shift-JIS(CP932)で保存されている**ため、編集時にUTF-8前提のツールを使うと
文字化けで破壊するリスクがある。編集手順の注意は
[`PROJECT_MAP.md`](./PROJECT_MAP.md) の「既知の注意点」を参照。

---

## 3. ブランチ命名(`docs/NAMING.md` より、汎用的に有効)

`{type}/{PascalCase summary}` 形式。

| 種別 | 用途 | 例 |
|---|---|---|
| `feature` | 新機能追加 | `feature/AddModel` |
| `fix` | バグ修正 | `fix/FixTexture` |
| `refactor` | リファクタリング | `refactor/UserService` |
| `hotfix` | 緊急修正 | `hotfix/FixCrashOnStart` |
| `docs` | ドキュメント修正 | `docs/UpdateReadme` |
| `test` | テスト追加・修正 | `test/AddUserServiceTest` |

---

## 4. 禁止パターン

- 意味のない省略(`tmp`, `data2`, `func`, `aaa` のような名前)
- 1文字変数(ループカウンタの `i`/`j`/`k` は許容)
- 実装の都合を晒す名前(`Manager2`, `NewModelLoader` のような後付け名)
