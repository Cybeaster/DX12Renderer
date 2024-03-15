#pragma once

#include "Texture.h"
#include "TexturesReader/TexturesParser.h"

#include <unordered_map>
#include <unordered_set>

class OCommandQueue;
class OEngine;
class OTextureManager : public IRenderObject
{
public:
	using TTexturesMap = std::unordered_map<string, unique_ptr<STexture>>;
	using TTexturesMapPath = std::unordered_map<wstring, STexture*>;
	OTextureManager(ID3D12Device* Device, OCommandQueue* CommandList);

	STexture* CreateTexture(string Name, wstring FileName);
	STexture* CreateTexture(const wstring& FileName);

	STexture* FindTextureByName(string Name) const;
	STexture* FindTextureByPath(wstring Path) const;
	STexture* FindOrCreateTexture(wstring FileName);
	STexture* FindOrCreateTexture(string Name, wstring FileName);

	TTexturesMap& GetTextures() { return Textures; }
	uint32_t GetNumTextures() const { return Textures.size(); }
	void LoadLocalTextures();
	void SaveLocalTextures();
	wstring GetName() override;
private:
	void AddTexture(unique_ptr<STexture> Texture);
	void RemoveAllTextures();
	void RemoveTexture(const string& Name);
	void RemoveTexture(const wstring& Path);
private:
	wstring Name = L"TextureManager";
	unique_ptr<OTexturesParser> Parser;
	ID3D12Device* Device;
	OCommandQueue* CommandQueue;
	inline static std::unordered_set<uint32_t> TexturesHeapIndicesTable = {};
	TTexturesMap Textures;
	TTexturesMapPath TexturesPath;
};
