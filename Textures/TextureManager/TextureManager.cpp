#include "TextureManager.h"

#include "../DDSTextureLoader/DDSTextureLoader.h"
#include "CommandQueue/CommandQueue.h"
#include "Exception.h"
#include "Logger.h"
#include "Settings.h"

#include <filesystem>
OTextureManager::OTextureManager(ID3D12Device* Device, OCommandQueue* Queue)
    : Device(Device), CommandQueue(Queue)
{
	CommandQueue->TryResetCommandList();
	LoadLocalTextures();
	CommandQueue->ExecuteCommandListAndWait();
}

void OTextureManager::LoadLocalTextures()
{
	auto path = std::filesystem::current_path();
	path /= SConfig::TexturesFolder;
	if (!exists(path))
	{
		LOG(Engine, Error, "Textures folder not found!");
		return;
	}
	for (auto& entry : std::filesystem::directory_iterator(path))
	{
		if (is_regular_file(entry))
		{
			const auto filename = entry.path().filename().wstring();
			const auto name = entry.path().stem().string();
			if (entry.path().extension() == L".dds")
			{
				CreateTexture(name, entry.path().wstring());
			}
		}
	}
}

STexture* OTextureManager::CreateTexture(string Name, wstring FileName)
{
	if (Textures.contains(Name))
	{
		LOG(Engine, Error, "Texture with this name already exists!");
		return nullptr;
	}

	auto texture = make_unique<STexture>(Name, FileName);
	texture->Name = Name;
	texture->FileName = FileName;
	texture->HeapIdx = Textures.size();
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
	                                                    CommandQueue->GetCommandList().Get(),
	                                                    texture->FileName.c_str(),
	                                                    texture->Resource,
	                                                    texture->UploadHeap));
	Textures[Name] = move(texture);
	LOG(Engine, Log, "Texture created: Name : {}, Path: {}", TO_STRING(Name), FileName);
	return Textures[Name].get();
}

STexture* OTextureManager::FindTexture(string Name) const
{
	if (!Textures.contains(Name))
	{
		LOG(Engine, Error, "Texture not found!");
		return nullptr;
	}
	return Textures.at(Name).get();
}