#pragma once
#include "Filters/FilterBase.h"
class OSobelFilter : public OFilterBase
{
public:
	OSobelFilter(ID3D12Device* Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format);
	OSobelFilter(const OSobelFilter& rhs) = delete;
	OSobelFilter& operator=(const OSobelFilter& rhs) = delete;

	CD3DX12_GPU_DESCRIPTOR_HANDLE OutputSRV() const;

	uint32_t GetNumSRVRequired() const override
	{
		return 2;
	}

	void BuildDescriptors() const override;
	void BuildResource() override;

	std::pair<bool, CD3DX12_GPU_DESCRIPTOR_HANDLE> Execute(SPSODescriptionBase* PSO, CD3DX12_GPU_DESCRIPTOR_HANDLE Input);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void SetIsEnabled(bool bIsEnabled)
	{
		bEnabled = bIsEnabled;
	}

	bool GetIsEnabled() const
	{
		return bEnabled;
	}

private:
	SDescriptorPair SRVHandle;
	SDescriptorPair UAVHandle;

	SResourceInfo Output;
	bool bEnabled = true;
};
