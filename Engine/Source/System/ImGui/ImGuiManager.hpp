#pragma once
#include <string>
#include <functional>
#include "Utility/Singleton/Singleton.hpp"

struct ImGuiContext;

namespace Engine::Graphics
{
	class DX12Device;
	class DX12RendererContext;
	class DX12DescriptorHeapManager;
} // Engine::Graphics

namespace Engine::System
{
	class Window;

	class ImGuiManager : public Engine::Utility::Singleton<ImGuiManager>
	{
		DECLARE_SINGLETON(ImGuiManager)
	public:
		// 初期化
		bool Initialize(Window& window, Engine::Graphics::DX12Device& device, Engine::Graphics::DX12RendererContext& context, Engine::Graphics::DX12DescriptorHeapManager& heapManager);
		// 終了処理
		void Finalize();

		// フレーム開始
		void NewFrame();
		// 登録されたUI関数を順に呼び出す
		void Update();
		// 描画データの確定とImGuiコマンドの発行
		void EndFrame();

		// デバッグUI描画関数を登録
		void AddDebugUI(std::function<void()> guiFunc, const std::string& key);
		// keyに対応する登録を解除
		void RemoveDebugUI(const std::string& key);
		// keyが現在登録されているか
		bool HasDebugUI(const std::string& key) const;
		// 全てのデバッグUIを解除する
		void ClearDebugUI();

		// WindowのWndProcから転送してもらう
		static LRESULT WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:

		// 登録済みのデバッグUI描画関数
		// 呼び出し順は不定（unordered_map）。表示順に依存するUIはkeyにソート用の
		// プレフィックスを含めるなどして呼び出し側で調整すること
		std::unordered_map<std::string, std::function<void()>> debugUIFunctions;

		// EndFrame()でコマンドリストを取得するための借用元
		Engine::Graphics::DX12RendererContext* rendererContext = nullptr;
		// EndFrame()のSetDescriptorHeapsに渡すヒープの借用元
		Engine::Graphics::DX12DescriptorHeapManager* heapManager = nullptr;

		ImGuiContext* imguiContext = nullptr; // 将来モジュールを分割する場合の保持用
		bool initialized = false;
	};
} // Engine::System