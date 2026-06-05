#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include "../RenderContext/DX12RendererContext.hpp"

namespace Engine::Graphics
{
	class DX12Renderer : public Engine::Utility::Singleton<DX12Renderer>
	{
		DECLARE_SINGLETON(DX12Renderer)
	public:

	private:

	};
}