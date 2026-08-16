# ノート
DirectX12の初期化クラス構成

## TODO: 
---

画像の描画

---

Engine
 └─ Source
     ├─ Framework
     ├─ pch
     │
     ├─ System        // OSやCPU寄りのシステム層（ウィンドウ、入力など）
     │   └─ Window
     │       └─ Window.hpp / .cpp
     │
     ├─ Graphics      // 独立！GPU・描画を担当するレイヤー
     │   ├─ Dx12Device.hpp / .cpp
     │   ├─ Dx12Render.hpp / .cpp
     │   └─ Dx12RenderContext.hpp / .cpp
     │
     └─ Utility

Window
 └ (OSのウィンドウハンドルを管理。Dx12側へ提供)

Dx12Device (基盤・ファクトリー層)
 ├ IDXGIFactory (アダプタの列挙、SwapChainの生成用)
 ├ ID3D12Device (リソースやパイプラインの生成コア)
 ├ Dx12CommandQueue (画面表示・全体同期用：Direct型)
 └ Dx12DescriptorHeapManager (RTV, DSV, CBV/SRV/UAVのプール)

Dx12Render (描画システム全体・パイプライン管理)
 ├ Dx12SwapChain (画面表示バッファの管理)
 ├ Dx12PipelineStateManager (PSOやRootSignatureの管理)
 └ Dx12RenderContext (描画実行のスレッドや単位ごとの文脈)

Dx12RenderContext
 ├ ID3D12CommandAllocator (フレームごとにリセットして使うメモリ)
 ├ ID3D12GraphicsCommandList (実際の描画コマンドを記録する場所)
 └ Dx12Fence (CPUとGPUの同期用)

## メモ
### DirectX 12の初期化クラス構成について
ファイル構成
DX12の中にDeviceとRendererとRendererContextのファイルを作成しそれぞれのファイルに.hppと.cppを作成する
Rendererはシングルトンで管理する
RendererはContextを管理するクラス
Contextは将来的にマルチスレッドにする

DirectX12の初期化について
DX12Device
デバイス(デバイスの生成)、デバック、ファクトリー
DX12RendererContext
コマンドリスト、コマンドキュー、スワップチェイン、コマンドアロケータ
ディスクリプタヒープ、レンダーターゲットビュー、深度ステンシルバッファ
フェンス

### RendererとContextの役割分担
```sh
最初はRendererとContextを分けずにやってみる
マルチスレッド化させるタイミングでRendererContextからRendererを切り離す
Rendererは管理だけにする
```

Rendererは画面出力・同期
* スワップチェーン (IDXGISwapChain)
* レンダーターゲットビュー (RTV) / 深度ステンシルビュー (DSV) のディスクリプタヒープ
* バックバッファリソース（画面の絵をためる場所）
* フェンス (ID3D12Fence) と WaitGPU() メソッド

コマンド関係をContext側にまとめる
* コマンドキュー (ID3D12CommandQueue)
* コマンドアロケータ (ID3D12CommandAllocator)
* コマンドリスト (ID3D12GraphicsCommandList)

---

RendererがContextを管理させる
Contextを取得したいときはRendererから取得する形にする

---

WaitGPUはcontext側にメソッド作って入れておく

AssimpよりもFBXSDK
データを持ってきやすい
Assimpのほうが一応楽なのであとでどっちでするか考える

## 破棄順序をちゃんと気にしておく
### DirectX 12 初期化の生成順番
1 DXGIファクトリの作成：デバイスの列挙やウィンドウ管理の土台となるオブジェクトを作成します。
2 デバイス（ID3D12Device）の作成：GPUと対話するためのコアとなるデバイスオブジェクトを生成します。
3 コマンドキュー（ID3D12CommandQueue）の作成：GPUに実行させたい命令列を投入するキュー（待ち行列）を生成します。
4 コマンドアロケータ（ID3D12CommandAllocator）の作成：描画コマンドをメモリ上に記録するための領域を確保します。
5 コマンドリスト（ID3D12GraphicsCommandList）の作成：実際に描画命令や状態遷移を記録するためのリストを生成します。
6 フェンス（ID3D12Fence）の作成：CPUとGPUの処理順序や同期（待ち合わせ）をとるためのオブジェクトを作成します。
7 スワップチェーン（IDXGISwapChain）の作成：画面に描画結果を表示（フリップ）するためのバッファ群を生成します。
8 デスクリプタヒープ（ID3D12DescriptorHeap）の作成：テクスチャやビューなどのリソースをGPUに認識させるためのディスクリプタを管理するヒープを生成します。
9 ルートシグネチャの作成：シェーダーに渡す定数やリソースのバインディング（結合）ルールを定義します。
10 パイプラインステート（PSO）の作成：頂点シェーダーやピクセルシェーダー、ブレンド設定などの状態をまとめたパイプラインステートオブジェクトを生成します。リソース（バッファ、テクスチャ）の作成：頂点データや定数バッファ、レンダーターゲットなどのメモリ領域をGPU上に確保します。

### DirectX 12 初期化の破棄順番
1 コマンド系オブジェクトの破棄
コマンドリストコマンドアロケータ、コマンドキューを解放します。
2 フェンス（Fence）の破棄
GPUの同期に使用したフェンスオブジェクトを解放します。
3 描画・パイプライン状態の破棄
パイプラインステート（PSO）、ルートシグネチャを解放します。
4 ディスクリプタヒープの破棄
RTV, DSV, CBV/SRV/UAVなどの各種ディスクリプタヒープを解放します。
5 リソースオブジェクトの破棄
レンダーターゲット、深度ステンシル、テクスチャ、頂点/インデックスバッファなどのResourceを解放します。
6 スワップチェーンの破棄
IDXGISwapChainを解放します。デバイス・ファクトリの破棄最後に、デバイス（ID3D12Device）とDXGIファクトリ（IDXGIFactory）を解放します。

## メモ
ENTT
テクスチャ側のリソース
リソース
メモリ割り当て情報
SRV用のディスクリプタスロット
テクスチャの幅

---

DescriptorHandleを引数で参照
DX12DescriptorHeapManager 参照
// DX12DescriptorHeapManager singletonで持ってくる

---

リソースの管理
スプライトコンポーネント（パラメータだけ、テクスチャの参照）
スプライトのパイプライン
スプライトのレンダラー

スプライト側にカラーを持たせる
画像が参照できない場合にカラーを表示する
スプライトのどこまで表示できるようにするか


コンポーネント思考でいくのかいかないのかどっちなんだい！
スプライトクラスを作るかENTTでやるか

ライブラリ入れるか入れないかどっちなんだい！

マルチスレッド

イニシャライズをboolを特定のエラーハンドルを作成する。
このIDはこれーとか
ID string

## シェーダー
VSMain（頂点シェーダ）
頂点の座標をWVP行列で変換して画面上の位置を決めています。UVは uvRect を使って「テクスチャのどの範囲を表示するか」に変換
PSMain（ピクセルシェーダ）
テクスチャから色をサンプリングして、SpriteComponent::color を掛け合わせています。フォールバックのホワイトテクスチャの場合は color がそのまま出力


スプライトのサイズ、位置、pivot、色の変更
入力関係
カメラ　２ｄ、３ｄ
モデルの描画 Assimp or FBX SDK
ライティング

後で
深度ソート
サウンド（miniaudio）
フォント


## 未完了・今後の予定

優先度高

EntityInspector  （ImGuiでEntity・Component確認）
モデルの描画      （Assimp or FBX SDK）
深度ソート

優先度中

ライティング・シャドウ
3Dカメラ操作（パン・オービット）
Triangleの削除（テスト用なので整理）

優先度低

サウンド（miniaudio）
フォント・テキスト描画
Assetsの自動読み込み
RenderImGuiの実装（KeyInput・MouseInput・PadInput）

設計として検討中

コマンドの記録とGPU送信の分離
AddComponent の空構造体対応の整理


今後
EntityInspectorのコンポーネントの登録をFrameWorkでやるのではなく別のクラスで行うようにする
その後モデル


## モデル
ModelVertex.hpp
役割: 頂点データの構造体定義のみ
持つもの:
  position   （位置）
  normal     （法線）
  uv         （テクスチャ座標）
  tangent    （接線）
  bitangent  （従法線）
Material
役割: 1つのマテリアルのデータを持つ
持つもの:
  diffuseTexture  （色テクスチャのID）
  color           （ベースカラー）
将来追加:
  normalMap       （法線マップ）
  roughnessMap    （粗さ）
  metallicMap     （金属度）
Mesh
役割: 1つのメッシュのGPUリソースを管理
持つもの:
  VertexBuffer
  IndexBuffer
  Material参照
やること:
  GPUリソースの生成・破棄
  Draw呼び出し
Model
役割: 複数のMeshをまとめたもの
持つもの:
  Mesh の配列
  モデル名
将来追加:
  アニメーション情報
  ボーン情報
ModelID
役割: 型安全なモデルの識別子
TextureIDと同じ設計
  using ModelID = uint64_t;
  static constexpr ModelID InvalidModelID = 0;
ModelLoader
役割: Assimpを使ってファイルを読み込んでModelを生成する
やること:
  aiScene からメッシュデータを取得
  頂点・インデックスデータを変換
  マテリアル・テクスチャ情報を取得
  Model を返す
ModelManager
役割: Modelのロード・キャッシュ・全破棄
TextureManagerと同じ設計
持つもの:
  unordered_map<ModelID, unique_ptr<Model>>  キャッシュ
やること:
  Load(path) → ModelID
  Get(id)    → Model*
  Finalize() → 全破棄
ModelRenderer
役割: Modelを描画する
持つもの:
  PSO
  RootSignature
  ConstantBuffer<ModelTransformBuffer>
やること:
  Initialize（PSO・RootSignature生成）
  Draw（cmdList・Model・カメラ行列を受け取って描画）
ModelRenderSystem
役割: ENTTのview<>でModelComponentを持つEntityを回してModelRendererを呼ぶ
SpriteRenderSystemと同じ設計
ModelComponent
役割: ENTTのコンポーネント、パラメータのみ
持つもの:
  ModelID   modelID
  bool      visible
依存関係の流れ
ModelLoader
    ↓ Model を生成
ModelManager
    ↓ Model を管理・提供
ModelRenderer
    ↓ Model を描画
ModelRenderSystem
    ↓ ENTT で回す

1. ModelVertex.hpp
2. Material.hpp / .cpp
3. Mesh.hpp / .cpp
4. Model.hpp / .cpp
5. ModelLoader.hpp / .cpp（ModelManagerが内包）
6. ModelManager.hpp / .cpp
7. ModelRenderer.hpp / .cpp
8. ModelRenderSystem.hpp / .cpp
9. ModelComponent.hpp