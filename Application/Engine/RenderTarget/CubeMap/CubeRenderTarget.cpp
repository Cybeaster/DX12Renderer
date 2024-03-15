
#include "CubeRenderTarget.h"

#include "Engine/Engine.h"

OCubeRenderTarget::OCubeRenderTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format, const DirectX::XMUINT2 Res)
    : ORenderTargetBase(Device, Width, Height, Format), Resolution(Res)
{
	Name = L"CubeRenderTarget";
	BuildViewport();
}

OCubeRenderTarget::OCubeRenderTarget(const SRenderTargetParams& Params, const DirectX::XMUINT2 Res)
    : ORenderTargetBase(Params), Resolution(Res)
{
	BuildViewport();
}

uint32_t OCubeRenderTarget::GetNumPassesRequired() const
{
	return 6;
}

void OCubeRenderTarget::BuildDepthStencilBuffer()
{
	OEngine::Get()->GetCommandQueue()->TryResetCommandList();
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = Resolution.x;
	depthStencilDesc.Height = Resolution.y;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = SRenderConstants::DepthBufferFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = SRenderConstants::DepthBufferFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	auto cmd = OEngine::Get()->GetCommandQueue()->GetCommandList().Get();
	CubeDepthStencilBuffer = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, cmd, &optClear);
	Device->CreateDepthStencilView(CubeDepthStencilBuffer.Resource.Get(), nullptr, DSVHandle.CPUHandle);

	OEngine::Get()->GetCommandQueue()->ExecuteCommandListAndWait();
}

void OCubeRenderTarget::BuildViewport()
{
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = static_cast<float>(Width);
	Viewport.Height = static_cast<float>(Height);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	ScissorRect = { 0, 0, Width, Height };
}

void OCubeRenderTarget::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}
	descriptor->SRVHandle.Offset(SRVHandle);
	descriptor->RTVHandle.Offset(RTVHandle, GetNumRTVRequired());
	descriptor->DSVHandle.Offset(DSVHandle);
	BuildDescriptors();
	BuildDepthStencilBuffer();
}

uint32_t OCubeRenderTarget::GetNumRTVRequired() const
{
	return 6;
}

uint32_t OCubeRenderTarget::GetNumDSVRequired() const
{
	return 1;
}
uint32_t OCubeRenderTarget::GetNumSRVRequired() const
{
	return 1;
}

SDescriptorPair OCubeRenderTarget::GetDSV(uint32_t SubtargetIdx) const
{
	return DSVHandle;
}

SDescriptorPair OCubeRenderTarget::GetRTV(uint32_t SubtargetIdx) const
{
	if(SubtargetIdx < RTVHandle.size())
	{
		return RTVHandle[SubtargetIdx];
	}

		LOG(Render, Error, "SubtargetIdx is out of range")
return {};
}

void OCubeRenderTarget::InitRenderObject()
{
	ORenderTargetBase::InitRenderObject();
	BuildResource();
}

void OCubeRenderTarget::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.DepthOrArraySize = GetNumRTVRequired();
	texDesc.MipLevels = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	RenderTarget = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OCubeRenderTarget::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	Device->CreateShaderResourceView(RenderTarget.Resource.Get(), &srvDesc, SRVHandle.CPUHandle);

	for (int i = 0; i < GetNumRTVRequired(); i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = Format;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;

		// Render target to ith element.
		rtvDesc.Texture2DArray.FirstArraySlice = i;

		// Only view one element of the array.
		rtvDesc.Texture2DArray.ArraySize = 1;

		// Create RTV to ith cubemap face.
		Device->CreateRenderTargetView(RenderTarget.Resource.Get(), &rtvDesc, RTVHandle[i].CPUHandle);
	}
}