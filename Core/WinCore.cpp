#include <Windows.h>
#include "DXCore.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WindowProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"PongDX12Class";

	if (!RegisterClassExW(&wcex)) {
		return -1; // Something went terribly wrong at registration
	}

	HWND hWnd = CreateWindowExW(
		0,
		wcex.lpszClassName,
		L"PongDX12",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(hWnd, nShowCmd);
	
	IDXGIFactory7* factory = CreateDXGIFactory();
	IDXGIAdapter1* adapter = GetHardwareAdapter(factory, true);


	MSG msg = { 0 };

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		{
			// WE ARE DOING DIRECTX12 RENDERING HERE BABY!!
		}
	}
	factory->Release();
	adapter->Release();
}
