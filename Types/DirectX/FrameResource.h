#pragma once
#include "MaterialConstants.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "ObjectConstants.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct SVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
};


struct SFrameResource
{
	SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT WaveVertexCount, UINT MaterialCount);

	SFrameResource(const SFrameResource&) = delete;

	SFrameResource& operator=(const SFrameResource&) = delete;

	~SFrameResource();

	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// We cannot update a cbuffer until the GPU is done processing the
	// commands that reference it. So each frame needs their own cbuffers.

	unique_ptr<OUploadBuffer<SPassConstants>> PassCB = nullptr;
	unique_ptr<OUploadBuffer<SObjectConstants>> ObjectCB = nullptr;
	unique_ptr<OUploadBuffer<SMaterialConstants>> MaterialCB = nullptr;

	// We cannot update a dynamic vertex buffer until the GPU is done processing
	// the commands that reference it.  So each frame needs their own.
	unique_ptr<OUploadBuffer<SVertex>> WavesVB = nullptr;

	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
};

inline SFrameResource::SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT WaveVertexCount, UINT MaterialCount)
{
	THROW_IF_FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	PassCB = make_unique<OUploadBuffer<SPassConstants>>(Device, PassCount, true);
	ObjectCB = make_unique<OUploadBuffer<SObjectConstants>>(Device, ObjectCount, true);
	WavesVB = make_unique<OUploadBuffer<SVertex>>(Device, WaveVertexCount, false);
	if (MaterialCount > 0)
	{
		MaterialCB = make_unique<OUploadBuffer<SMaterialConstants>>(Device, MaterialCount, true);
	}
	else
	{
		LOG(Warning, "Material count is 0");
	}
}

inline SFrameResource::~SFrameResource()
{
}