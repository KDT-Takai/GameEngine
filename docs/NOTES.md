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
 └ Dx12RenderContext (★描画実行のスレッドや単位ごとの文脈)

Dx12RenderContext
 ├ ID3D12CommandAllocator (フレームごとにリセットして使うメモリ)
 ├ ID3D12GraphicsCommandList (実際の描画コマンドを記録する場所)
 └ Dx12Fence (CPUとGPUの同期用)

## メモ
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

WaitGPUはcontext側にメソッド作って入れておく

AssimpよりもFBXSDK
データを持ってきやすい
Assimpのほうが一応楽なのであとでどっちでするか考える

### 破棄順序をちゃんと気にしておく
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