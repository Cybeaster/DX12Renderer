#pragma once
#include "Filters/FilterBase.h"

#include <Types.h>
#include <d3dx12.h>
#include <wrl/client.h>

struct SConstantBlurSettings
{
	int BlurRadius;
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

class OBlurFilter : public OFilterBase
{
public:
	OBlurFilter(ID3D12Device* Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format);
	OBlurFilter(const OBlurFilter& rhs) = delete;
	OBlurFilter& operator=(const OBlurFilter& rhs) = delete;

	void OutputTo(SResourceInfo* Destination);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void Execute(
	    const SPSODescriptionBase* HorizontalBlurPSO,
	    const SPSODescriptionBase* VerticalBlurPSO,
	    SResourceInfo* Input);
	uint32_t GetNumSRVRequired() const override
	{
		return 4;
	}

	void SetParameters(float InSigma, uint32_t InBlurCount)
	{
		Sigma = InSigma;
		BlurCount = InBlurCount;
	}

private:
	vector<float> CalcGaussWeights(float Sigma) const;

	void BuildDescriptors() const override;
	void BuildResource() override;

private:
	const uint32_t MaxBlurRadius = 5;

	SDescriptorPair SRV0Handle;
	SDescriptorPair UAV0Handle;

	SDescriptorPair SRV1Handle;
	SDescriptorPair UAV1Handle;

	SResourceInfo BlurMap0;
	SResourceInfo BlurMap1;
	unique_ptr<OUploadBuffer<SConstantBlurSettings>> Buffer;

	float Sigma = 2.5;
	uint32_t BlurCount = 1;
};
