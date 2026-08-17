#include "pch/pch.h"
#include "RuntimeShaderLoader.hpp"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

namespace Engine::Graphics
{
	bool RuntimeShaderLoader::Load(const ShaderLoadDesc& desc, ID3DBlob** ppBlob)
	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		HRESULT hr = D3DCompileFromFile(
			desc.path.c_str(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // .hlsl 内での #include を有効にする
			desc.entryPoint.c_str(),
			desc.target.c_str(),
			compileFlags,
			0,
			ppBlob,
			errorBlob.GetAddressOf()
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				LOG_ERROR(static_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			else
			{
				// ファイルが見つからない場合など
				LOG_ERROR("シェーダファイルが見つかりません: "
					+ std::string(desc.path.begin(), desc.path.end()));
			}
			return false;
		}

		return true;
	}
} // Engine::Graphics