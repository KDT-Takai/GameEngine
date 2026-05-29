# ノート
DirectX12の初期化クラス構成

System
 ├ Window
 ├ Graphics
 │   ├ Device
 │   ├ CommandQueue
 │   ├ SwapChain
 │   ├ DescriptorHeap
 │   └ Fence
 ├ Renderer
 │   ├ Mesh
 │   ├ Texture
 │   ├ Material
 │   ├ PipelineState
 │   └ RootSignature
 ├ Scene
 │   ├ Camera
 │   └ Entity
 └ ResourceManager