#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include "../RendererContext/DX12RendererContext.hpp"

namespace Engine::Graphics
{
	class DX12Renderer : public Engine::Utility::Singleton<DX12Renderer>
	{
		DECLARE_SINGLETON(DX12Renderer)
	public:
		// 初期化
		bool Initialize(HWND hwnd, UINT width, UINT height);
		// 終了
		void Finalize();

		// フレーム処理
		bool BeginFrame();
		bool EndFrame();

		// DX12RendererContextの取得用メソッド
		DX12RendererContext* GetContext() const { return context.get(); }
	private:
		// レンダラーコンテキスト
		std::unique_ptr<DX12RendererContext> context;
	};
} // Engine::Graphics