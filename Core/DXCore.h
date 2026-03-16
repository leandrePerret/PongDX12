#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

void EnableDebugLayer();
IDXGIFactory7* CreateDXGIFactory();
IDXGIAdapter1* GetHardwareAdapter(IDXGIFactory7* factory, bool requestHighPerformanceAdapter);
ID3D12Device* CreateDevice(IDXGIAdapter1* adapter);
ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* device);
void FlushCommandQueue(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence);
DXGI_SWAP_CHAIN_DESC1 CreateSwapChainDesc(UINT width, UINT height, UINT bufferCount);
IDXGISwapChain1* CreateSwapChain(IDXGIFactory7* factory, HWND hWnd, ID3D12CommandQueue* commandQueue, DXGI_SWAP_CHAIN_DESC1* swapChainDesc);
extern UINT fenceValue;
const UINT FrameCount = 2;
extern ID3D12Resource* renderTargets[FrameCount];
