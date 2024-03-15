#pragma once
#include "CommandQueue/CommandQueue.h"
#include "DirectX/DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"

class OFilterBase : public ORenderObjectBase
{
public:
	virtual ~OFilterBase() = default;

	OFilterBase(ID3D12Device* Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format)
	    : Device(Device), Queue(Other), Width(Width), Height(Height), Format(Format)
	{
	}
	void InitRenderObject();

	template<typename T>
	static T* CreateFilter(ID3D12Device* _Device, OCommandQueue* Queue, UINT _Width, UINT _Height, DXGI_FORMAT _Format)
	{
		auto newFilter = new T(_Device, Queue, _Width, _Height, _Format);
		newFilter->InitRenderObject();
		return newFilter;
	}

	virtual void OnResize(UINT NewWidth, UINT NewHeight);
	wstring GetName() override
	{
		return FilterName;
	}
protected:
	virtual void BuildDescriptors() const = 0;
	virtual void BuildResource() = 0;

	OCommandQueue* Queue = nullptr;
	ID3D12Device* Device = nullptr;
	uint32_t Width = 0;
	uint32_t Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	wstring FilterName;
};

inline void OFilterBase::InitRenderObject()
{
	BuildResource();
}

inline void OFilterBase::OnResize(UINT NewWidth, UINT NewHeight)
{
	if (NewWidth != Width || NewHeight != Height)
	{
		Width = NewWidth;
		Height = NewHeight;

		BuildResource();
		BuildDescriptors();
	}
}