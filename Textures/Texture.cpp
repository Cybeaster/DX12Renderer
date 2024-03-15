#include "Texture.h"
D3D12_SRV_DIMENSION STextureViewType::GetViewType(const string& Type)
{
	if (Type == Texture2D)
	{
		return D3D12_SRV_DIMENSION_TEXTURE2D;
	}
	if (Type == TextureCube)
	{
		return D3D12_SRV_DIMENSION_TEXTURECUBE;
	}
	return D3D12_SRV_DIMENSION_UNKNOWN;
}

string STextureViewType::GetViewString(const D3D12_SRV_DIMENSION Type)
{
	if (Type == D3D12_SRV_DIMENSION_TEXTURE2D)
	{
		return Texture2D;
	}
	if (Type == D3D12_SRV_DIMENSION_TEXTURECUBE)
	{
		return TextureCube;
	}
	return "Unknown";
}

vector<string> STextureViewType::GetTextureTypes()
{
	return { Texture2D, TextureCube };
}

D3D12_SHADER_RESOURCE_VIEW_DESC STexture::GetSRVDesc() const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Resource.Resource->GetDesc().Format;
	srvDesc.ViewDimension = STextureViewType::GetViewType(ViewType);
	srvDesc.Texture2D.MipLevels = Resource.Resource->GetDesc().MipLevels;
	return srvDesc;
}