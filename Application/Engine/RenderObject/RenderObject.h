#pragma once

#include "DirectX/ObjectConstants.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"
#include "Logger.h"
#include "Statics.h"

struct IDescriptor
{
	virtual ~IDescriptor() = default;
};

struct SDescriptorPair
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle = {};
	uint32_t Index = UINT32_MAX;
};

struct SDescriptorResourceData
{
	ID3D12DescriptorHeap* Heap;
	UINT Size;
	UINT Count;
};

struct TDescriptorHandle
{
	void Offset(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPUHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& OutGPUHandle, uint32_t& OutIndex)
	{
		OutCPUHandle = CPUHandle;
		OutGPUHandle = GPUHandle;
		OutIndex = CurrentOffset;
		CurrentOffset++;
		CPUHandle.Offset(1, DescSize);

		if (GPUHandle.ptr != 0)
		{
			GPUHandle.Offset(1, DescSize);
		}
		Check();
	}

	void Offset(SDescriptorPair& OutHandle)
	{
		Offset(OutHandle.CPUHandle, OutHandle.GPUHandle, OutHandle.Index);
	}

	void Offset(vector<SDescriptorPair>& OutHandle, uint32_t NumDesc)
	{
		for (uint32_t i = 0; i < NumDesc; i++)
		{
			auto pair = Offset();
			if (i >= OutHandle.size())
			{
				OutHandle.push_back(pair);
			}
			else
			{
				OutHandle[i] = pair;
			}
		}
	}

	void Offset(vector<SDescriptorPair>& OutHandle)
	{
		for (auto& handle : OutHandle)
		{
			Offset(handle.CPUHandle, handle.GPUHandle, handle.Index);
		}
	}

	SDescriptorPair Offset(const uint32_t Value = 1)
	{
		const auto oldCPU = CPUHandle;
		const auto oldGPU = GPUHandle;
		const auto oldOffset = CurrentOffset;
		CurrentOffset += Value;
		CPUHandle.Offset(Value, DescSize);
		if (GPUHandle.ptr != 0)
		{
			GPUHandle.Offset(Value, DescSize);
		}
		Check();
		return { oldCPU, oldGPU, oldOffset };
	}

	void Check() const
	{
		if (CurrentOffset > MaxOffset)
		{
			LOG(Render, Error, "Descriptor heap overflow");
		}

		if (!bIsInitized)
		{
			LOG(Render, Error, "Descriptor heap not initized");
		}

		if (MaxOffset == 0)
		{
			LOG(Render, Error, "Descriptor heap max count not initized");
		}
	}

	void Init(SDescriptorResourceData Data)
	{
		bIsInitized = true;
		MaxOffset = Data.Count;
		DescSize = Data.Size;
		CPUHandle = Data.Heap->GetCPUDescriptorHandleForHeapStart();
		if (Data.Heap->GetDesc().Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			GPUHandle = Data.Heap->GetGPUDescriptorHandleForHeapStart();
		}
	}

	bool bIsInitized = false;
	uint32_t DescSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
	uint32_t CurrentOffset = 0;
	uint32_t MaxOffset = 0;
};

struct SRenderObjectDescriptor : IDescriptor
{
	void Init(SDescriptorResourceData SRV, SDescriptorResourceData RTV, SDescriptorResourceData DSV);

	TDescriptorHandle SRVHandle;
	TDescriptorHandle RTVHandle;
	TDescriptorHandle DSVHandle;
};

struct SPassConstantsData
{
	int32_t StartIndex = -1;
	int32_t EndIndex = INT32_MAX;
	OUploadBuffer<SPassConstants>* Buffer = nullptr;
};

class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual void BuildDescriptors(IDescriptor* Descriptor){};
	virtual void InitRenderObject(){};
	virtual uint32_t GetNumSRVRequired() const {return 0;}
	virtual uint32_t GetNumRTVRequired() const { return 0; }
	virtual uint32_t GetNumDSVRequired() const { return 0; }
	virtual uint32_t GetNumPassesRequired() const { return 0; }
	virtual void UpdatePass(const SPassConstantsData& Data) {}
	virtual void Update(const UpdateEventArgs& Event) {}
	virtual TUUID GetID() { return {}; }
	virtual void SetID(TUUID ID) {}
	virtual wstring GetName() { return { L"UNIMPLEMENTED" }; }
};

class ORenderObjectBase : public IRenderObject
{
public:
	void SetID(TUUID ID) override
	{
		this->ID = ID;
	}
	TUUID GetID() override
	{
		return ID;
	}

protected:
	TUUID ID = {};
};