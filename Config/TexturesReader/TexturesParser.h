#pragma once
#include "ConfigReader.h"
#include "Texture.h"

struct STexture;
class OTexturesParser : public OConfigReader
{
public:
	OTexturesParser(const string& ConfigPath)
	    : OConfigReader(ConfigPath)
	{
		LoadConfig(FileName);
	}

	void AddTexture(STexture* Texture);
	void AddTextures(const vector<STexture*>& Textures);
	vector<STexture*> LoadTextures();
private:
	ETextureType GetTextureType(const string& Type);
};
