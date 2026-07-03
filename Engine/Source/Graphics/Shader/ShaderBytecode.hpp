#pragma once
#include <vector>
#include <cstdint>
#include <d3d12.h>

namespace Engine::Graphics
{
	struct ShaderBytecode
	{
		std::vector<uint8_t> data;

		bool IsValid() const { return !data.empty(); }

		const void* GetBufferPointer() const { return data.data(); }
		SIZE_T      GetBufferSize()    const { return data.size(); }

		D3D12_SHADER_BYTECODE GetD3D12Bytecode() const
		{
			return { GetBufferPointer(), GetBufferSize() };
		}
	};

}