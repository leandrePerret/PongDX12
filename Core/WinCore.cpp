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
		CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(hWnd, nShowCmd);
	
	IDXGIFactory7* factory = CreateDXGIFactory();
	IDXGIAdapter1* adapter = GetHardwareAdapter(factory, true);
	ID3D12Device* device = CreateDevice(adapter);
	ID3D12CommandQueue* commandQueue = CreateCommandQueue(device);
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = CreateSwapChainDesc(WindowWidth, WindowHeight, FrameCount);
	IDXGISwapChain3* swapChain = CreateSwapChain(factory, hWnd, commandQueue, &swapChainDesc);
	ID3D12DescriptorHeap* rtvHeap = CreateDescriptorHeap(device);
	CreateRenderTargets(device, swapChain, rtvHeap);
	ID3D12CommandAllocator* commandAllocator = CreateCommandAllocator(device);
	ID3D12GraphicsCommandList* commandList = CreateCommandList(device, commandAllocator);
	ID3D12Fence* fence = CreateFence(device);
	MSG msg = { 0 };

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		{
			// WE ARE DOING DIRECTX12 RENDERING HERE BABY!!
			// First we wait for the GPU to finish whatever it was doing
			FlushCommandQueue(commandQueue, fence);

			// Second we rest the command allocator and command list to prepare for recording commands for the next frame
			commandAllocator->Reset();
			commandList->Reset(commandAllocator, nullptr);

			// Third we get which buffer 0 or 1 is the back buffer
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			// Fourth we record commands to clear the back buffer to a specific color
			// 4.1 We need to find where the current back buffer is in the render target view heap
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
			UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			rtvHandle.ptr += backBufferIndex * rtvDescriptorSize; // The idea here is that we switch of buffer if we are on buffer 1

			// 4.2 We create a barrier to make a transition from the present state to the render target state
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = renderTargets[backBufferIndex];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			commandList->ResourceBarrier(1, &barrier);

			// 4.3 We clear the back buffer to a specific color
			const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			// 4.4 We create a barrier to make a transition from the render target state to the present state
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

			commandList->ResourceBarrier(1, &barrier);

			// Fifth the recording is done. We close the list
			commandList->Close();

			// Sixth we execute the command list to render the frame
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);

			// Seventh we present the frame
			swapChain->Present(1, 0);

		}
	}
	factory->Release();
	adapter->Release();
}
