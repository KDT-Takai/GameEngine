#pragma once
#include "Graphics/DX12/Type.hpp"
#include "Graphics/Shader/IShaderLoader.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/ConstantBuffer/TransformBuffer.hpp"
#include <dxgi1_6.h>
#include <DirectXMath.h>

namespace Engine::Graphics
{
	struct Vertex
	{
		float position[3];
		float color[4];
	};

	class Triangle
	{
	public:
		bool Initialize(
			ID3D12Device* device,
			IShaderLoader& shaderLoader,
			DXGI_FORMAT rtvFormat,
			DXGI_FORMAT dsvFormat
		);
		void Finalize();

		// 毎フレーム呼んで定数バッファを更新する
		void SetWVP(const DirectX::XMMATRIX& wvp);

		void Draw(ID3D12GraphicsCommandList* cmdList);

	private:
		bool CreateRootSignature(ID3D12Device* device);
		bool CreatePipelineState(
			ID3D12Device* device,
			IShaderLoader& shaderLoader,
			DXGI_FORMAT rtvFormat,
			DXGI_FORMAT dsvFormat
		);
		bool CreateVertexBuffer(ID3D12Device* device);

		dx12::ComPtr<ID3D12RootSignature> rootSignature;
		dx12::ComPtr<ID3D12PipelineState> pipelineState;
		dx12::ComPtr<ID3D12Resource>      vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW          vertexBufferView{};

		// 定数バッファ
		ConstantBuffer<TransformBuffer> constantBuffer;
	};
} // Engine::Graphics