#pragma once

// Window
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <wrl.h>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <DirectXMath.h>
#include <comdef.h>
#include <d3dx12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

// DirectXTex
#ifdef _DEBUG
#pragma comment(lib, "..\\packages\\directxtex_desktop_win10.2026.5.8.1\\native\\lib\\x64\\Debug\\DirectXTex.lib")
#else
#pragma comment(lib, "..\\packages\\directxtex_desktop_win10.2026.5.8.1\\native\\lib\\x64\\Release\\DirectXTex.lib")
#endif

// STL
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <deque>

#include <string>
#include <string_view>
#include <sstream>

#include <memory>
#include <optional>
#include <variant>
#include <functional>

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

#include <algorithm>
#include <ranges>
#include <execution>

#include <filesystem>
#include <fstream>
#include <iostream>

#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <future>
#include <condition_variable>

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <tuple>
#include <span>
#include <bitset>
#include <bit>
#include <limits>
#include <numbers>

#include <type_traits>
#include <concepts>
#include <coroutine>

#include <utility>
#include <initializer_list>

#include <format>

// log
#include "Utility/Logger/Log.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"