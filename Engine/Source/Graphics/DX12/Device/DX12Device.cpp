#include "pch/pch.h"
#include "DX12Device.hpp"

namespace Engine::Graphics::DX12
{
	bool DX12Device::Initialize() {
		if (!CreateDevice()) {
			LOG_ERROR("デバイスの作成に失敗");
			return false;
		}
		if (!CreateDebugDevice()) {
			LOG_ERROR("デバッグデバイスの作成に失敗");
		}
		if (!CreateAdapter()) {
			LOG_ERROR("アダプターの作成に失敗");
			return false;
		}
		if (!CreateFactory()) {
			LOG_ERROR("ファクトリーの作成に失敗");
			return false;
		}
		return true;
	}

	void DX12Device::Finalize() {
		if (device) device.Reset();
		if (adapter) adapter.Reset();
		if (factory) factory.Reset();
		if (debugDevice) debugDevice.Reset();
	}

	bool DX12Device::CreateDevice() {
		auto hr = CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()));
		return SUCCEEDED(hr);
	}

	bool DX12Device::CreateDebugDevice() {
#if defined(DEBUG) || defined(_DEBUG)
	auto hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDevice));
	if (FAILED(hr)) {
		LOG_ERROR("デバッグレイヤーの取得に失敗");
		return true;
	}
#endif
	return true;
	}

	bool DX12Device::CreateAdapter() {
	}

	bool DX12Device::CreateFactory() {
	}
}