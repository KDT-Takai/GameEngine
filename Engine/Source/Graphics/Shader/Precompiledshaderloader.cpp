#include "pch/pch.h"
#include "PrecompiledShaderLoader.hpp"

//namespace Engine::Graphics
//{
//	bool PrecompiledShaderLoader::Load(const ShaderLoadDesc& desc, ID3DBlob** ppBlob)
//	{
//		// .cso をバイナリとして読み込み、Blob に格納する
//		// D3DReadFileToBlob は d3dcompiler.lib に含まれる
//		// HRESULT hr = D3DReadFileToBlob(desc.path.c_str(), ppBlob);
//		// if (FAILED(hr))
//		// {
//		//     LOG_ERROR(".csoファイルの読み込みに失敗: " + std::string(desc.path.begin(), desc.path.end()));
//		//     return false;
//		// }
//		// return true;
//	}
//}