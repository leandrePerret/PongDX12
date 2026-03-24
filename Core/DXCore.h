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
IDXGISwapChain3* CreateSwapChain(IDXGIFactory7* factory, HWND hWnd, ID3D12CommandQueue* commandQueue, DXGI_SWAP_CHAIN_DESC1* swapChainDesc);
ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device);
void CreateRenderTargets(ID3D12Device* device, IDXGISwapChain3* swapChain, ID3D12DescriptorHeap* rtvHeap);
ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device* device);
ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* allocator);
ID3D12Fence* CreateFence(ID3D12Device* device);
extern UINT fenceValue;
const UINT WindowWidth = 1920;
const UINT WindowHeight = 1080;
const UINT FrameCount = 2;
extern ID3D12Resource* renderTargets[FrameCount];
