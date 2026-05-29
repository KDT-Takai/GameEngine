#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <windows.h>
#include <tchar.h>
#include <spdlog/spdlog.h>
#include "Framework/Framework.hpp"

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//defined(DEBUG) || defined(_DEBUG)

	Framework::Create();
	if (!Framework::Get().Initialize(1280, 720, "DirectX12"))
	{
		return -1;
	}
	Framework::Get().Run();
	Framework::Get().Finalize();

	return 0;
}