#pragma once
#include "Engine/RenderObject/RenderObject.h"
#include "Engine/UploadBuffer/UploadBuffer.h"

class OCommandQueue;
struct SRenderTargetParams
{
	ID3D12Device* Device;
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
};

class ORenderTargetBase : public ORenderObjectBase
{
public:
	ORenderTargetBase(ID3D12Device* Device,
	                  UINT Width, UINT Height,
	                  DXGI_FORMAT Format)
	    : Width(Width), Height(Height), Format(Format), Device(Device)
	{
	}

	ORenderTargetBase(const SRenderTargetParams& Params)
	    : Width(Params.Width), Height(Params.Height), Format(Params.Format), Device(Params.Device)
	{
	}

	ORenderTargetBase(UINT Width, UINT Height);

	virtual void BuildDescriptors() = 0;
	virtual void BuildResource() = 0;

	virtual SResourceInfo* GetResource() = 0;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumRTVRequired() const override;
	uint32_t GetNumDSVRequired() const override;
	void CopyTo(ORenderTargetBase* Dest, const OCommandQueue* CommandQueue);
	virtual SDescriptorPair GetSRV(uint32_t SubtargetIdx = 0) const;
	virtual SDescriptorPair GetRTV(uint32_t SubtargetIdx = 0) const;
	virtual SDescriptorPair GetDSV(uint32_t SubtargetIdx = 0) const;
	virtual void InitRenderObject();

	TUUID GetID() override;
	void SetID(TUUID) override;
	void SetViewport(ID3D12GraphicsCommandList* List) const;
	virtual void PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList, uint32_t SubtargetIdx = 0);

	void UnsetRenderTarget(OCommandQueue* CommandQueue);
	wstring GetName() override
	{
		return Name;
	}
protected:
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	LONG Width = 0;
	LONG Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D12Device* Device = nullptr;
	unordered_set<uint32_t> PreparedTaregts;
	wstring Name = L"";
};

class OOffscreenTexture : public ORenderTargetBase
{
public:
	OOffscreenTexture(ID3D12Device* Device,
	                  UINT Width, UINT Height,
	                  DXGI_FORMAT Format);

	OOffscreenTexture(const OOffscreenTexture& rhs) = delete;
	OOffscreenTexture& operator=(const OOffscreenTexture& rhs) = delete;

	~OOffscreenTexture() override;

	SDescriptorPair GetSRV(uint32_t SubtargetIdx = 0) const override { return SRVHandle; }
	SDescriptorPair GetRTV(uint32_t SubtargetIdx = 0) const override { return RTVHandle; }
	uint32_t GetNumRTVRequired() const override { return 1; }
	uint32_t GetNumSRVRequired() const override { return 1; }

	void BuildDescriptors(IDescriptor* Descriptor) override;
	void OnResize(UINT NewWidth, UINT NewHeight);
	void InitRenderObject() override;

protected:
	void BuildDescriptors() override;
	void BuildResource() override;

public:
	SResourceInfo* GetResource() override;

private:
	SDescriptorPair SRVHandle;
	SDescriptorPair RTVHandle;

	// Two for ping-ponging the textures.
	SResourceInfo RenderTarget;
};
