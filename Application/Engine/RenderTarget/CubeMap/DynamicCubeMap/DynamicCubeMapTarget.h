#pragma once
#include "Camera/Camera.h"
#include "Engine/RenderTarget/CubeMap/CubeRenderTarget.h"

class ODynamicCubeMapRenderTarget final : public OCubeRenderTarget
{
public:
	ODynamicCubeMapRenderTarget(const SRenderTargetParams& Params, DirectX::XMFLOAT3 Center, const DirectX::XMUINT2& Res)
	    : OCubeRenderTarget(Params, Res), Position(Center) {Name = L"DynamicCubeMap";}

	void InitRenderObject() override;
	void UpdatePass(const SPassConstantsData& Data) override;

	D3D12_GPU_VIRTUAL_ADDRESS GetPassConstantAddresss(int Index) const //TODO propagate to base class
	{
		auto& pass = PassConstants[Index];
		return pass.Buffer->GetGPUAddress() + pass.StartIndex * Utils::CalcBufferByteSize(sizeof(SPassConstants));
	}

private:
	DirectX::XMFLOAT3 Position;
	vector<unique_ptr<OCamera>> Cameras;
	vector<SPassConstantsData> PassConstants;
};
