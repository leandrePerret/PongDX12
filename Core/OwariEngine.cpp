#include "OwariEngine.h"

OwariEngine::OwariEngine()
{
}

OwariEngine::~OwariEngine()
{
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