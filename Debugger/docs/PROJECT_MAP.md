# プロジェクトマップ (AI/開発者向け)

このファイルは、AIエージェント(Claude Code等)がこのリポジトリを素早く把握できるように、
ファイル構成・各モジュールの役割・現在の実装状況をまとめたものです。
コードを読む前にまずこのファイルを見れば、どこに何があるか一発で分かることを目的としています。

## ソリューション構成

| プロジェクト | 種別 | Release同梱 | 内容 |
|---|---|---|---|
| `App` | Application (.exe) | ○ | エントリポイント(`main.cpp`)のみ。実体はEngine/Debuggerに依存 |
| `Engine` | Static Library | ○ | ゲームエンジン本体(描画・ECS・アセット管理など)。Release版に必須のコードのみ |
| `Debugger` | Static Library | ✕ (**Debugのみ**) | デバッグ・開発支援ツール(EntityInspector, AssetBrowserなど)。最終版には含めない |
| `docs` | ソリューションフォルダ | - | `NOTES.md`(設計メモ), `NAMING.md`(命名規則) |

### Debugger方式について
- `App`→`Debugger`のプロジェクト参照は `Condition="'$(Configuration)'=='Debug'"` でDebug構成のみ有効
- `.sln`側もDebuggerの`Release|x64/x86`に`.Build.0`行を持たせていないため、Releaseソリューションビルドではそもそもビルド対象外
- `Engine`側(`Framework.hpp`/`Framework.cpp`)でDebugger機能の呼び出しは全て `#ifdef _DEBUG` で囲ってあるので、Release構成(`NDEBUG`定義)ではシンボル参照自体が発生せず、Debugger.libなしでもリンクが通る
- **新しくデバッグ専用機能を追加するときのルール**: (1) 実体の `.cpp`/`.hpp` を `Debugger.vcxproj` の `ClCompile`/`ClInclude` に追加(物理ファイルはどこでもよいが、既存踏襲なら `Engine\Source\System\...` 配下に置いて相対パス参照) (2) 呼び出し側は `#ifdef _DEBUG` で囲む

## ディレクトリ構成と役割

```
GameEngine/
├─ App/                      実行用プロジェクト
│  ├─ main.cpp                エントリポイント(Framework::Initialize/Run/Finalizeを呼ぶだけ)
│  └─ Assets/                 ゲームアセット(Models/*.mdl [+変換元の*.fbx], Textures/*.png, Test/*.png)
│
├─ Engine/                    エンジン本体(Release同梱)
│  ├─ Source/
│  │  ├─ Framework/           初期化・メインループ統括(Framework.hpp/.cpp)
│  │  ├─ Graphics/
│  │  │  ├─ DX12/             D3D12デバイス・レンダラー・デスクリプタヒープ等の低レベルラッパー
│  │  │  ├─ Model/            独自バイナリ`.mdl`のモデル読み込み・描画(BinaryModelLoader/ModelManager/ModelRenderer/Mesh/Material。2026-09-05にAssimp版ModelLoaderを撤去、Assimp依存なし)
│  │  │  ├─ Sprite/           2Dスプライト描画
│  │  │  ├─ Primitive/        Triangle(テスト用、削除予定 by NOTES.md)
│  │  │  ├─ Texture/          テクスチャ読み込み・管理
│  │  │  ├─ Shader/           シェーダーローダー(Runtime/Precompiled)
│  │  │  ├─ Component/        ECSコンポーネント(Transform/Sprite/Model)
│  │  │  └─ ConstantBuffer/   定数バッファラッパー
│  │  ├─ System/
│  │  │  ├─ Assets/           AssetManager(名前→ID解決、ディレクトリフォールバック機能あり)
│  │  │  ├─ Assets/AssetBrowser/  ★Debugger行き(ImGuiでアセット一覧表示)
│  │  │  ├─ EntityInspector/  ★Debugger行き(ImGuiでEntity/Component確認・編集)
│  │  │  ├─ Camera/           カメラコンポーネント・システム(Ortho/Perspective)
│  │  │  ├─ ECS/              entt registryラッパー
│  │  │  ├─ Input/            キーボード・マウス・パッド入力
│  │  │  ├─ ImGui/            ImGuiManager(初期化・NewFrame/EndFrame)
│  │  │  └─ Screen/           仮想解像度管理
│  │  ├─ Utility/
│  │  │  ├─ Logger/           spdlogラッパー。コンソール出力(Release同梱)。ファイル出力はDebugger側で追加
│  │  │  ├─ Singleton/        シングルトン基底クラス
│  │  │  └─ EngineContext/    サービスロケータ的な仕組み
│  │  └─ pch/                 プリコンパイル済みヘッダ(Windows/D3D12/STL/ImGuiを一括include)
│  ├─ Assets/Shader/          HLSLシェーダー(Model.hlsl, Sprite.hlsl, Triangle.hlsl)
│  └─ external/                ImGui, entt, SpdLog等のサードパーティ(Assimpは撤去済み、変換ツール側のみで使用)
│
├─ Debugger/                  ★デバッグ専用プロジェクト(Release非同梱)
│  ├─ Source/Logging/         DevLogSink(ファイルログ出力の実体)
│  ├─ Logs/latest.log         ★実行するたびに上書きされるログファイル。AIはここを読めば実行結果が分かる
│  └─ docs/
│     ├─ CONVENTIONS.md       ファイル構成・命名規則(実コードから観測した実態)
│     └─ PROJECT_MAP.md       このファイル
│
└─ docs/
   ├─ NOTES.md                設計メモ・TODO(開発者の生の思考メモ、優先度付きTODOあり)
   └─ NAMING.md                命名規則
```

## 既知の注意点(ハマりどころ)

### 1. ソースファイルの文字コード(CP932/Shift-JIS)
`Engine/Source` 配下の多くの `.cpp`/`.hpp` は **Shift-JIS(CP932)** で保存されている(UTF-8ではない)。
UTF-8前提で読み書きするツールでこれらのファイルを編集すると、日本語コメント・ログ文字列が
**復元不能な文字化け(`�` 置換文字)に破壊される**ことがある(実際に一度事故った)。

- **編集前に必ず、対象行がASCIIのみか確認する**。日本語コメント行を含む範囲を編集する場合は、
  `[System.Text.Encoding]::GetEncoding(932)` でバイト列を明示的にデコード/エンコードしてから
  文字列置換する(PowerShellで `ReadAllBytes` → `GetString(932)` → `.Replace()` → `GetBytes(932)` → `WriteAllBytes`)。
- ASCII部分のみを対象にした最小限のリテラル置換にとどめ、正規表現の `.*?` や `(?s)` (DOTALL) は
  絶対に複数行・複数関数をまたいで使わない(誤って関数丸ごと削除する事故が起きた実績あり)。
- `.vcxproj` / `.sln` / `.filters` はUTF-8(BOMなし)なので通常のEditツールで問題ない。

### 2. ログファイル
`Debugger/Logs/latest.log` は実行するたびに上書きされる(Debug版のみ生成、Releaseでは出力されない)。
AIがアプリの実行結果を確認したいときは、ユーザーに実行してもらった後、このファイルを直接読めばよい
(貼り付けてもらう必要はない)。ただしログ本文の日本語部分は上記の理由で読み取り時に文字化けして
見えるので、ファイル名・行番号・数値・英語文字列などから状況を判断する。

### 3. モデルの座標変換規約
DirectXMathは行ベクトル規約(`v' = v * M`)。HLSL側でこれと整合させるには
`#pragma pack_matrix(row_major)` が必要(`Model.hlsl`で対応済み)。これが無いとC++側の行列と
HLSL側のデフォルト(column_major)がズレて、変換が壊れる(過去に発生した不具合)。

### 4. モデル読み込み(`.mdl`バイナリ、Assimp非依存)
ランタイムはAssimpを使わず、独自バイナリ形式`.mdl`を`BinaryModelLoader`で読むだけ(2026-09-05、
Assimp版`ModelLoader`を撤去)。`.fbx`から`.mdl`への変換は別リポジトリ[ModelConverter](https://github.com/ShaF-u/ModelConverter)の
CLIツール(`Tools/ModelConverter.exe`)で事前に行う。新規モデルはこのツールを通さないと
エンジンで読み込めない(直接FBXを読むフォールバックは無い)。
FBXのマテリアル変換時の注意点(テクスチャ格納チャンネルが`aiTextureType_DIFFUSE`とは限らず
`BASE_COLOR`や`EMISSIVE`に入ることがある等)はModelConverter側の`ProcessMaterial`の話であり、
このリポジトリのコードには存在しない。

## 現在の実装状況(docs/NOTES.mdより抜粋・整理)

### 実装済み
- DirectX12基盤(Device/Renderer/RendererContext/DescriptorHeap)
- ECS(entt)基盤、TransformComponent/SpriteComponent/ModelComponent/CameraComponent
- スプライト描画、モデル描画(独自バイナリ`.mdl`経由、テクスチャ解決含む。Assimpはランタイム非依存)
- EntityInspector(ImGui、Debugger行き)
- AssetManager(拡張子別ローダー登録、ディレクトリフォールバック解決)
- ファイルログ出力(`Debugger/Logs/latest.log`、DevLogSink経由、Debugのみ)
- Debug/Release分離の仕組み(Debuggerプロジェクト)

### 優先度: 高(次にやること候補)
- 深度ソート
- ライティング・シャドウ
- 3Dカメラ操作の改善(パン・オービット)
- Triangle(テスト用プリミティブ)の削除

### 優先度: 低
- サウンド(miniaudio)
- フォント・テキスト描画
- Assetsの自動読み込みの仕上げ
- RenderImGuiの実装(KeyInput/MouseInput/PadInputの可視化)

### 設計として検討中
- コマンド記録とGPU送信の分離
- AddComponentの汎用対応の整理

詳細な設計思想・議論の経緯は `docs/NOTES.md` を参照(こちらは開発者の生メモなので情報量は多いが整理されていない)。
