//
// Created by Cybea on 27/02/2024.
//

#include "Shader.h"

void SShaderDefinition::TypeFromString(const string& Other)
{
	if (Other == "Vertex")
	{
		ShaderType = EShaderLevel::VertexShader;
	}
	else if (Other == "Pixel")
	{
		ShaderType = EShaderLevel::PixelShader;
	}
	else if (Other == "Compute")
	{
		ShaderType = EShaderLevel::ComputeShader;
	}
	else if (Other == "Geometry")
	{
		ShaderType = EShaderLevel::GeometryShader;
	}
	else if (Other == "Hull")
	{
		ShaderType = EShaderLevel::HullShader;
	}
	else if (Other == "Domain")
	{
		ShaderType = EShaderLevel::DomainShader;
	}
	else
	{
		WIN_LOG(Default, Error, "Unknown shader type: {}", TO_STRING(Other));
	}
}

void OShader::Init(const SShaderDefinition& Info, const ComPtr<IDxcBlob>& Blob, D3D12_INPUT_LAYOUT_DESC Desc)
{
	ShaderBlob = Blob;
	ShaderInfo = Info;
	LayoutDesc = Desc;
}