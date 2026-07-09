#include "pch/pch.h"
#include "Triangle.hpp"

namespace Engine::Graphics
{
	bool Triangle::Initialize(
		ID3D12Device* device,
		IShaderLoader& shaderLoader,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT dsvFormat)
	{
		if (!CreateRootSignature(device))
		{
			LOG_ERROR("RootSignatureの生成に失敗");
			return false;
		}
		if (!CreatePipelineState(device, shaderLoader, rtvFormat, dsvFormat))
		{
			LOG_ERROR("PipelineStateの生成に失敗");
			return false;
		}
		if (!CreateVertexBuffer(device))
		{
			LOG_ERROR("VertexBufferの生成に失敗");
			return false;
		}

		LOG_INFO("Triangleの初期化に成功");
		return true;
	}

	void Triangle::Finalize()
	{
		vertexBuffer.Reset();
		pipelineState.Reset();
		rootSignature.Reset();
	}

	void Triangle::Draw(ID3D12GraphicsCommandList* cmdList)
	{
		cmdList->SetGraphicsRootSignature(rootSignature.Get());
		cmdList->SetPipelineState(pipelineState.Get());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}

	bool Triangle::CreateRootSignature(ID3D12Device* device)
	{
		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.NumParameters = 0;
		desc.pParameters = nullptr;
		desc.NumStaticSamplers = 0;
		desc.pStaticSamplers = nullptr;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		dx12::ComPtr<ID3DBlob> signatureBlob;
		dx12::ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			signatureBlob.GetAddressOf(),
			errorBlob.GetAddressOf()
		);
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				LOG_ERROR(static_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			return false;
		}

		hr = device->CreateRootSignature(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf())
		);
		if (FAILED(hr))
		{
			LOG_ERROR("RootSignatureの生成に失敗: {0:x}", HR_LOG(hr));
			return false;
		}

		return true;
	}

	bool Triangle::CreatePipelineState(
		ID3D12Device* device,
		IShaderLoader& shaderLoader,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT dsvFormat)
	{
		dx12::ComPtr<ID3DBlob> vsBlob;
		dx12::ComPtr<ID3DBlob> psBlob;

		// 頂点シェーダ
		ShaderLoadDesc vsDesc;
		vsDesc.path = L"../Engine/Assets/Shader/Triangle.hlsl";
		vsDesc.entryPoint = "VSMain";
		vsDesc.target = "vs_5_0";

		// ピクセルシェーダ
		ShaderLoadDesc psDesc;
		psDesc.path = L"../Engine/Assets/Shader/Triangle.hlsl";
		psDesc.entryPoint = "PSMain";
		psDesc.target = "ps_5_0";

		if (!shaderLoader.Load(vsDesc, vsBlob.GetAddressOf()))
		{
			LOG_ERROR("頂点シェーダのコンパイルに失敗");
			return false;
		}
		if (!shaderLoader.Load(psDesc, psBlob.GetAddressOf()))
		{
			LOG_ERROR("ピクセルシェーダのコンパイルに失敗");
			return false;
		}

		// 入力レイアウト
		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// ラスタライザ
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthClipEnable = TRUE;

		// ブレンド
		D3D12_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// 深度ステンシル
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = rootSignature.Get();
		psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
		psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
		psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = blendDesc;
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = rtvFormat;
		psoDesc.DSVFormat = dsvFormat;
		psoDesc.SampleDesc.Count = 1;

		HRESULT hr = device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf())
		);
		if (FAILED(hr))
		{
			LOG_ERROR("PipelineStateObjectの生成に失敗: {0:x}", HR_LOG(hr));
			return false;
		}

		return true;
	}

	bool Triangle::CreateVertexBuffer(ID3D12Device* device)
	{
		Vertex vertices[] =
		{
			{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		};

		const UINT bufferSize = sizeof(vertices);

		D3D12_HEAP_PROPERTIES heapProp{};
		heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufferDesc{};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = bufferSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		HRESULT hr = device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(vertexBuffer.GetAddressOf())
		);
		if (FAILED(hr))
		{
			LOG_ERROR("VertexBufferの生成に失敗: {0:x}", HR_LOG(hr));
			return false;
		}

		// 頂点データをバッファへ書き込む
		void* mapped = nullptr;
		hr = vertexBuffer->Map(0, nullptr, &mapped);
		if (FAILED(hr))
		{
			LOG_ERROR("Map failed: {}", static_cast<uint32_t>(hr));
//			LOG_ERROR("VertexBufferのマップに失敗: {}", HR_LOG(hr));
			return false;
		}
		memcpy(mapped, vertices, bufferSize);
		vertexBuffer->Unmap(0, nullptr);

		// 頂点バッファビューの設定
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = bufferSize;
		vertexBufferView.StrideInBytes = sizeof(Vertex);

		return true;
	}
}