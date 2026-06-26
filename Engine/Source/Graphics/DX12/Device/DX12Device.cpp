#include "pch/pch.h"
#include "DX12Device.hpp"

namespace Engine::Graphics
{
	bool DX12Device::Initialize() {
		if (!EnableDebugLayer()) {
			LOG_ERROR("デバッグレイヤーの有効化に失敗");
			// デバッグは必須ではないためfalseは返さない
		}
		if (!CreateFactory()) {
			LOG_ERROR("ファクトリーの作成に失敗");
			return false;
		}
		if (!CreateAdapter()) {
			LOG_ERROR("アダプターの作成に失敗");
			return false;
		}
		if (!CreateDevice()) {
			LOG_ERROR("デバイスの作成に失敗");
			return false;
		}
		if (!CreateDebugDevice()) {
			LOG_ERROR("デバッグデバイスの作成に失敗");
		}
		LOG_INFO("DX12Deviceの初期化に成功");
		return true;
	}

	void DX12Device::Finalize() {
#if defined(DEBUG) || defined(_DEBUG)
		if (debugDevice) {
			debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		}
#endif
		if (debugDevice) debugDevice.Reset();
		if (device) device.Reset();
		if (adapter) adapter.Reset();
		if (factory) factory.Reset();
	}

	bool DX12Device::EnableDebugLayer() {
#if defined(DEBUG) || defined(_DEBUG)
		// デバッグレイヤーの有効化
		dx12::Debug debugController;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));
		if (FAILED(hr)) {
			LOG_ERROR("デバッグの有効化に失敗");
			return false;
		}
		debugController->EnableDebugLayer();
#endif
		return true;
	}

	bool DX12Device::CreateFactory() {
		UINT factoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		auto hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(factory.GetAddressOf()));
		if (FAILED(hr)) {
			LOG_ERROR("ファクトリーの作成に失敗");
			return false;
		}
		return true;
	}

	bool DX12Device::CreateAdapter() {
		// 高パフォーマンスGPUを優先して取得
		auto hr = factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.GetAddressOf()));
		if (FAILED(hr)) {
			LOG_ERROR("高パフォーマンスGPUの取得に失敗,デフォルトアダプターを使用");
			dx12::ComPtr<IDXGIAdapter1> adapter1;
			// 古いGPU用にEnumAdapters1でアダプターを取得
			hr = factory->EnumAdapters1(0, adapter1.GetAddressOf());
			if (FAILED(hr)) {
				LOG_ERROR("アダプターの取得に失敗");
				return false;
			}
			// IDXGIAdapter1からIDXGIAdapter4へキャスト
			hr = adapter1.As(&adapter);
			if (FAILED(hr)) {
				LOG_ERROR("IDXGIAdapter4へのキャストに失敗");
				return false;
			}
		}
		// アダプター情報をログに出力
#if defined(DEBUG) || defined(_DEBUG)
		DXGI_ADAPTER_DESC3 desc{};
		if (SUCCEEDED(adapter->GetDesc3(&desc))) {
			char name[128]{};
			WideCharToMultiByte(CP_ACP, 0, desc.Description, -1, name, sizeof(name), nullptr, nullptr);
			LOG_INFO("使用するアダプター: " + std::string(name));
		}
#endif
		return true;
	}

	bool DX12Device::CreateDevice() {
		auto hr = D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(device.GetAddressOf()));
		if (FAILED(hr)) {
			LOG_ERROR("デバイスの作成に失敗");
			hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(device.GetAddressOf()));
			if (FAILED(hr)) {
				// フューチャーレベルを下げても失敗した場合
				LOG_ERROR("デバイスの作成に失敗");
				return false;
			}
			// フューチャーレベルを下げて成功した場合
			// フューチャーレベルを更新しておく
			featureLevel = D3D_FEATURE_LEVEL_12_0;
		}
		return true;
	}

	bool DX12Device::CreateDebugDevice() {
#if defined(DEBUG) || defined(_DEBUG)
		auto hr = device->QueryInterface(IID_PPV_ARGS(debugDevice.GetAddressOf()));
		if (FAILED(hr)) {
			LOG_ERROR("デバッグデバイスの作成に失敗");
			return false;
		}
#endif
		return true;
	}
} // Engine::Graphics