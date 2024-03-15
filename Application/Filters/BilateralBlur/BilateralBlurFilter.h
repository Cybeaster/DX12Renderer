#pragma once
#include "Filters/FilterBase.h"

class OBilateralBlurFilter : public OFilterBase
{
public:
	struct SBilateralBlur
	{
		float SpatialSigma;
		float IntensitySigma;
		int KernelRadius;
	};

	struct SBufferConstants
	{
		uint32_t TextureWidth;
		uint32_t TextureHeight;
	};

	OBilateralBlurFilter(ID3D12Device* Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void OutputTo(SResourceInfo* Destination) const;
	void BuildDescriptors() const override;
	void BuildResource() override;
	void Execute(const SPSODescriptionBase* PSO,
	             SResourceInfo*);

	void Execute(SResourceInfo*);
	uint32_t GetNumSRVRequired() const override
	{
		return 4;
	}

	void SetSpatialSigma(float Value)
	{
		SpatialSigma = Value;
	}
	void SetIntensitySigma(float Value)
	{
		IntensitySigma = Value;
	}
	void SetBlurCount(int32_t Value)
	{
		BlurCount = Value;
	}


private:
	SDescriptorPair BlurOutputSrvHandle;
	SDescriptorPair BlurOutputUavHandle;
	SDescriptorPair BlurInputSrvHandle;
	SDescriptorPair BlurInputUavHandle;

	TUploadBuffer<SBilateralBlur> BlurBuffer;
	TUploadBuffer<SBufferConstants> BufferConstants;

	SResourceInfo InputTexture;
	SResourceInfo OutputTexture;
	float SpatialSigma;
	float IntensitySigma;
	int32_t BlurCount;
};
