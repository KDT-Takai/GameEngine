# ノート
DirectX12の初期化クラス構成

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