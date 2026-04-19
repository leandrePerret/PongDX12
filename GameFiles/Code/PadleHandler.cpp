#include "OMath.h"
#include "DXCore.h"

Vertex LeftPaddle[4] = {
	{-1.f,0.3f,0.f},
	{-0.95f,0.3f,0.f},
	{-1.f,-0.3f,0.f},
	{-0.95f,-0.3f,0.f}
};

Vertex RightPaddle[4] = {
	{0.95f,0.3f,0.f}, 
	{1.f,0.3f,0.f},
	{0.95f,-0.3f,0.f},
	{1.f,-0.3f,0.f}
};

ID3D12Resource* LeftPaddleBuffer = CreateVertexBuffer(device, LeftPaddle, sizeof(LeftPaddle));