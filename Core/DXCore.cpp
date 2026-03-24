#include "DXCore.h"
UINT fenceValue;
ID3D12Resource* renderTargets[FrameCount] = { nullptr };

void EnableDebugLayer() {
	//Enable the D3D12 debug layer
#if defined(_DEBUG)
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->Release();
	}
#endif
}

IDXGIFactory7* CreateDXGIFactory() {
	// Create a DXGI Factory which will be used to create other DXGI objects such as adapters and swap chains
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; // Permit to create a debug factory by switching this flag
#endif

	IDXGIFactory7* factory = nullptr;
	HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) {
		return nullptr;
	}

	return factory;
}

IDXGIAdapter1* GetHardwareAdapter(IDXGIFactory7* factory, bool requestHighPerformanceAdapter) {
	// Get an appropriate adapter (GPU)

	// First we try to find the best adapter that supports Direct3D 12.
	IDXGIAdapter1* adapter = nullptr;
	if (requestHighPerformanceAdapter) {
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)); ++adapterIndex) {
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
				return adapter; // Found a suitable adapter
			}

			adapter->Release(); // Release the adapter if not suitable to free memory
		}
	}

	// If we didn't find a high-performance adapter, we can try to find any adapter that supports Direct3D 12.
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex) {
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			return adapter; // Found a suitable adapter
		}
		adapter->Release(); // Release the adapter if not suitable to free memory
	}
	return nullptr;	// No suitable adapter found
}

ID3D12Device* CreateDevice(IDXGIAdapter1* adapter) {
	// Create a device which represents the display adapter and is used to create resources
	ID3D12Device* device = nullptr;
	HRESULT hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device)
	);
	if (FAILED(hr)) {
		return nullptr;
	}
	return device;
}

ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* device) {
	// Create a command queue to submit commands to the GPU
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
	return commandQueue;
}
 
ID3D12Fence* CreateFence(ID3D12Device* device) {
	// Create a fence for GPU-CPU synchronization
	ID3D12Fence* fence = nullptr;
	HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		return nullptr;
	}
	return fence;
}

HANDLE CreateEventHandle() {
	// Create a generic Win32 event
	// Parameters : Security (nullptr), ManualReset (FALSE), InitialState (FALSE), Name (nullptr)
	return CreateEvent(nullptr, FALSE, FALSE, nullptr);
	
}

void FlushCommandQueue(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence) {
	// Advance the fence value to mark commands up to this point to make the CPU wait until the GPU has completed processing
	fenceValue++;
	commandQueue->Signal(fence, fenceValue);
	if (fence->GetCompletedValue() < fenceValue) {
		HANDLE eventHandle = CreateEventHandle();
		fence->SetEventOnCompletion(fenceValue, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

DXGI_SWAP_CHAIN_DESC1 CreateSwapChainDesc(UINT width, UINT height, UINT bufferCount) {
	// Create a swap chain description if width and height put as 0 will use the window's width and height
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	return swapChainDesc;
}

IDXGISwapChain3* CreateSwapChain(IDXGIFactory7* factory, HWND hWnd, ID3D12CommandQueue* commandQueue, DXGI_SWAP_CHAIN_DESC1* swapChainDesc) {
	// Create the swap chain
	IDXGISwapChain1* tempSwapChain = nullptr;
	HRESULT hr = factory->CreateSwapChainForHwnd(
		commandQueue,
		hWnd,
		swapChainDesc,
		nullptr,
		nullptr,
		&tempSwapChain
	);
	if (FAILED(hr)) {
		return nullptr;
	}

	IDXGISwapChain3* swapChain = nullptr;
	hr = tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
	tempSwapChain->Release();

	if(FAILED(hr)) {
		return nullptr;
	}
	return swapChain;
}

ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device) {
	// Create a descriptor heap for render target views which means memory allocation for descriptors
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeap* rtvHeap = nullptr;
	HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	if (FAILED(hr)) {
		return nullptr;
	}
	return rtvHeap;
}

void CreateRenderTargets(ID3D12Device* device, IDXGISwapChain3* swapChain, ID3D12DescriptorHeap* rtvHeap) {
	// Create render target views for each frame in the swap chain and store them in an array
	// An RTV is a descriptor that permit the GPU to know how to access a resource as a render target (a texture that can be rendered to)
	// We ash how big a descriptor is in the heap to know how much to move the handle for each render target view
	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT n = 0; n < FrameCount; n++) {
		// Get the back buffer from the swap chain and create a render target view for it
		swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
		device->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize; // Move the handle to the next descriptor in the heap to avoid overwriting the previous one
	}
}

ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device* device) {
	// Create a command allocator which is used to allocate memory for command lists
	ID3D12CommandAllocator* commandAllocator = nullptr;
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr)) {
		return nullptr;
	}
	return commandAllocator;
}

ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* allocator) {
	// Create a command list which is used to record commands that will be submitted to the GPU
	ID3D12GraphicsCommandList* commandList = nullptr;
	HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&commandList));
	if (FAILED(hr)) {
		return nullptr;
	}
	commandList->Close(); // Command lists are created in the recording state. We need to close it before we can reset it to record commands.
	return commandList;
}