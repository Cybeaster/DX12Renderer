#pragma once
#include "Engine/RenderTarget/RenderTarget.h"

class OCubeRenderTarget : public ORenderTargetBase
{
public:
	OCubeRenderTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format, const DirectX::XMUINT2 Res);
	OCubeRenderTarget(const SRenderTargetParams& Params, DirectX::XMUINT2 Res);
	OCubeRenderTarget(const OCubeRenderTarget& rhs) = delete;
	OCubeRenderTarget& operator=(const OCubeRenderTarget& rhs) = delete;

	~OCubeRenderTarget() = default;

	D3D12_VIEWPORT& GetViewport() { return Viewport; }
	D3D12_RECT& GetScissorRect() { return ScissorRect; }

	void BuildDescriptors(IDescriptor* Descriptor) override;

	uint32_t GetNumRTVRequired() const override;
	uint32_t GetNumDSVRequired() const override;
	uint32_t GetNumSRVRequired() const override;

	SDescriptorPair GetDSV(uint32_t SubtargetIdx) const override;
	SDescriptorPair GetRTV(uint32_t SubtargetIdx) const override;
	void InitRenderObject() override;
	vector<SDescriptorPair>& GetRTVHandle() { return RTVHandle; }
	SDescriptorPair& GetDSVHandle() { return DSVHandle; }
	SDescriptorPair& GetSRVHandle() { return SRVHandle; }
	uint32_t GetNumPassesRequired() const override;
	SResourceInfo* GetResource() override { return &RenderTarget; }

protected:
	DirectX::XMUINT2 Resolution;
	void BuildDepthStencilBuffer();

private:
	void BuildViewport();
	void BuildResource() override;
	void BuildDescriptors() override;
	SResourceInfo RenderTarget;
	SResourceInfo CubeDepthStencilBuffer;

	SDescriptorPair SRVHandle;
	SDescriptorPair DSVHandle;
	vector<SDescriptorPair> RTVHandle;
};
