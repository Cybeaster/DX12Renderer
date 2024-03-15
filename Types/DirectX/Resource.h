#pragma once
#include "DXHelper.h"

class IRenderObject;
struct SResourceInfo
{
	void Init(IRenderObject* InContext, D3D12_RESOURCE_STATES InState)
	{
		Context = InContext;
		CurrentState = InState;
	}

	D3D12_RESOURCE_STATES CurrentState;
	ComPtr<ID3D12Resource> Resource;
	IRenderObject* Context;
};
