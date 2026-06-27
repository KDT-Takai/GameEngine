# ノート
DirectX12の初期化クラス構成

TODO: NAMING.mdにnamespaceを追加する

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

## 次の手順
ディスクリプタヒープの管理
IMGUIの追加