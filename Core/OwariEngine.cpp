#include "OwariEngine.h"

OwariEngine::OwariEngine()
{
}

OwariEngine::~OwariEngine()
{
	FlushCommandQueue(commandQueue, fence);

	for (int i = 0; i < FrameCount; i++) {
		if (renderTargets[i]) {
			renderTargets[i]->Release();
			renderTargets[i] = nullptr;
		}
	}

	factory->Release();
	adapter->Release();
	device->Release();
	commandQueue->Release();
	swapChain->Release();
	rtvHeap->Release();
	commandAllocator->Release();
	commandList->Release();
	fence->Release();
}

OwariEngine::OwariEngine(HWND windowHandle, int width, int height)
{
	Initialize(windowHandle, width, height);
}

bool OwariEngine::Initialize(HWND windowHandle, int width, int height) {
	// Initialize DirectX 12 resources
	factory = CreateDXGIFactory();
	adapter = GetHardwareAdapter(factory, true);
	device = CreateDevice(adapter);
	commandQueue = CreateCommandQueue(device);
	swapChainDesc = CreateSwapChainDesc(width, height, FrameCount);
	swapChain = CreateSwapChain(factory, windowHandle, commandQueue, &swapChainDesc);
	rtvHeap = CreateDescriptorHeap(device);
	CreateRenderTargets(device, swapChain, rtvHeap);
	commandAllocator = CreateCommandAllocator(device);
	commandList = CreateCommandList(device, commandAllocator);
	fence = CreateFence(device);
	return true;
}

void OwariEngine::BeginFrame() {
	// First we wait for the GPU to finish whatever it was doing
	FlushCommandQueue(commandQueue, fence);

	// Second we rest the command allocator and command list to prepare for recording commands for the next frame
	commandAllocator->Reset();
	commandList->Reset(commandAllocator, nullptr);

	// Third we get which buffer 0 or 1 is the back buffer
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	// Fourth we create a barrier to make a transition from the present state to the render target state
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTargets[backBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);
}

void OwariEngine::ClearScreen(float r, float g, float b, float a) {
	// We record commands to clear the back buffer to a specific color
	// We need to find where the current back buffer is in the render target view heap
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	rtvHandle.ptr += backBufferIndex * rtvDescriptorSize; // The idea here is that we switch of buffer if we are on buffer 1


	// We clear the back buffer to a specific color
	const float clearColor[] = { r, g, b, a };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void OwariEngine::EndFrame() {
	// We create a barrier to make a transition from the render target state to the present state
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTargets[backBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);

	// The recording is done. We close the list
	commandList->Close();

	// We execute the command list to render the frame
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);

	// We present the frame
	swapChain->Present(1, 0);
}