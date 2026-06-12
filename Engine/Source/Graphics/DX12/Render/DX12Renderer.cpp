#include "pch/pch.h"
#include "DX12Renderer.hpp"

namespace Engine::Graphics
{
	bool DX12Renderer::Initialize(HWND hwnd, UINT width, UINT height)
	{
		context = std::make_unique<DX12RendererContext>();
		if (!context->Initialize(hwnd, width, height))
		{
			LOG_ERROR("DX12ReendererContext‚Ì‰Šú‰»‚ÉŽ¸”s");
			return false;
		}

		LOG_INFO("DX12Renderer ‚Ì‰Šú‰»‚É¬Œ÷");
		return true;
	}

	void DX12Renderer::Finalize()
	{
		if (context)
		{
			context->Finalize();
			context.reset();
		}
	}

	bool DX12Renderer::BeginFrame()
	{
		return context->BeginFrame();
	}

	bool DX12Renderer::EndFrame()
	{
		return context->EndFrame();
	}
}