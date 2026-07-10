#pragma once
#include <d3d12.h>
#include <string>

namespace Engine::Graphics
{
	struct ShaderLoadDesc
	{
		std::wstring path;        // HLSLパス
		std::string  entryPoint;  // エントリーポイント名
		std::string  target;      // シェーダモデル
	};

	class IShaderLoader
	{
	public:
		virtual ~IShaderLoader() = default;

		virtual bool Load(const ShaderLoadDesc& desc, ID3DBlob** ppBlob) = 0;
	};
} // Engine::Graphics