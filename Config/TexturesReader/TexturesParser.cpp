//
// Created by Cybea on 24/02/2024.
//

#include "TexturesParser.h"

#include "../../Textures/Texture.h"

#include <ranges>
using namespace boost::property_tree;

void OTexturesParser::AddTexture(STexture* Texture)
{
	auto& items = PTree.get_child("Textures");
	auto name = Texture->Name;
	auto path = WStringToUTF8(Texture->FileName);
	bool hasfound = false;

	for (auto& val : items | std::views::values)
	{
		if (val.get<string>("Name") == name)
		{
			hasfound = true;
			auto& texNode = val;
			texNode.put("Path", path);
			texNode.put("ViewDimensions", Texture->ViewType);
		}
	}

	if (!hasfound)
	{
		ptree texNode;
		texNode.put("Name", name);
		texNode.put("Path", path);
		texNode.put("ViewDimensions", Texture->ViewType);
		items.push_back(std::make_pair("", texNode));
	}

	write_json(FileName, PTree);
}

void OTexturesParser::AddTextures(const vector<STexture*>& Textures)
{
	for (auto tex : Textures)
	{
		AddTexture(tex);
	}
}

vector<STexture*> OTexturesParser::LoadTextures()
{
	vector<STexture*> textures;
	auto& items = PTree.get_child("Textures");
	for (auto& val : items | std::views::values)
	{
		auto tex = new STexture();
		tex->Name = val.get<string>("Name");
		tex->FileName = UTF8ToWString(val.get<string>("Path"));
		tex->ViewType = val.get<string>("ViewDimensions");
		tex->Type = GetTextureType(GetAttribute(val,"Type"));
		textures.push_back(tex);
	}
	return textures;
}

ETextureType OTexturesParser::GetTextureType(const string& Type)
{
	if (Type == "Diffuse")
	{
		return ETextureType::Diffuse;
	}
	 if (Type == "Normal")
	{
		return ETextureType::Normal;
	}
	 if (Type == "Roughness")
	{
		return ETextureType::Roughness;
	}
	 if (Type == "Height")
	{
		return ETextureType::Height;
	}
	if(Type == "Occlusion")
	{
		return ETextureType::Occlusion;
	}

	LOG(Config, Error, "Texture type not found! {}", TEXT(Type))
	return ETextureType::Diffuse;

}