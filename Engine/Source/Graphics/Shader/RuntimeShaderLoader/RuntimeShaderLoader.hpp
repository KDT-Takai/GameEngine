#pragma once
#include "../IShaderLoader/IShaderLoader.hpp"

namespace Engine::Graphics
{
	class RuntimeShaderLoader : public IShaderLoader
	{
	public:
		bool Load(const ShaderLoadDesc& desc, ID3DBlob** ppBlob) override;
	};
} // Engine::Graphics