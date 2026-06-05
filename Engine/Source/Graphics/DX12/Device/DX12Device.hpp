#pragma once
#include "../Type.hpp"
#include "Utility/Singleton/Singleton.hpp"

namespace Engine::Graphics
{
	class DX12Device : public Engine::Utility::Singleton<DX12Device>
	{
		DECLARE_SINGLETON(DX12Device)
	public:
		// 初期化
		bool Initialize();
		// 終了
		void Finalize();

		// デバイスのゲッター
		dx12::Device GetDevice() const { return device; }
		// デバッグデバイスのゲッター
		dx12::DebugDevice GetDebugDevice() const { return debugDevice; }
		// アダプターのゲッター
		dx12::Adapter GetAdapter() const { return adapter; }
		// ファクトリーのゲッター
		dx12::Factory6 GetFactory() const { return factory; }

	private:
		// デバッグ層の有効化
		bool EnableDebugLayer();
		// ファクトリーの作成
		bool CreateFactory();
		// アダプターの作成
		bool CreateAdapter();
		// デバイスの作成
		bool CreateDevice();
		// デバッグデバイスの作成
		bool CreateDebugDevice();
	
		// デバイスのレベル
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
		// デバイス本体
		dx12::Device device;
		// デバッグデバイス
		dx12::DebugDevice debugDevice;
		// アダプター
		dx12::Adapter adapter;
		// ファクトリー
		dx12::Factory6 factory;
	};
}