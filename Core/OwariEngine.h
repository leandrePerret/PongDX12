#pragma once
#include "DXCore.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class OwariEngine
{
private:
	IDXGIFactory7* factory;
	IDXGIAdapter1* adapter;
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	IDXGISwapChain3* swapChain;
	ID3D12DescriptorHeap* rtvHeap;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence;
	UINT backBufferIndex;

public:
	OwariEngine();
	~OwariEngine();
	OwariEngine(const OwariEngine&) = delete;
	OwariEngine(HWND windowHandle, int width, int height);
	bool Initialize(HWND windowHandle, int width, int height);
	void BeginFrame();
	void ClearScreen(float r, float g, float b, float a);
	void EndFrame();
	void DrawRectangle(float x, float y, float width, float height);
};

struct GameObject
{
	ID3D12Resource* vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
};