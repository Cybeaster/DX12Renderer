#pragma once
#include "Engine/RenderObject/RenderObject.h"
#include "Windows.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>
#pragma optimize("", off)
struct STexture
{
	virtual ~STexture() = default;
	string Name;
	wstring FileName;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;

	int64_t HeapIdx = -1;
	virtual D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = Resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = Resource->GetDesc().MipLevels;
		return srvDesc;
	}
};

struct SCubeMapTexture final : public STexture
{
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const override
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = Resource->GetDesc().MipLevels;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		srvDesc.Format = Resource->GetDesc().Format;
		return srvDesc;
	}
};
