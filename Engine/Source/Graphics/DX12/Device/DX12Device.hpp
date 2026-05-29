#pragma once
#include "../Type.hpp"

namespace Engine::Graphics::DX12
{
	class DX12Device
	{
	public:
		// 初期化
		bool Initialize();
		// 終了
		void Finalize();
		// ゲッター
		dx12::DevicePtr GetDevice() const { return device; }
		// デバッグデバイスのゲッター
		dx12::DebugDevicePtr GetDebugDevice() const { return debugDevice; }
		// アダプターのゲッター
		dx12::AdapterPtr GetAdapter() const { return adapter; }
		// ファクトリーのゲッター
		dx12::Factory6Ptr GetFactory() const { return factory; }
		// デバイスの作成
		bool CreateDevice();
		// デバッグデバイスの作成
		bool CreateDebugDevice();
		// アダプターの作成
		bool CreateAdapter();
		// ファクトリーの作成
		bool CreateFactory();
	private:
		// デバイスのレベル
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
		// デバイス
		dx12::DevicePtr device;
		// デバッグデバイス
		dx12::DebugDevicePtr debugDevice;
		// アダプター
		dx12::AdapterPtr adapter;
		// ファクトリー
		dx12::Factory6Ptr factory;
	};
}