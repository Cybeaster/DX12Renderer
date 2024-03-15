#include "CommandQueue.h"

#include "DirectX/ShaderTypes.h"
#include "Logger.h"

#include <Exception.h>

OCommandQueue::OCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> Device, D3D12_COMMAND_LIST_TYPE Type)
    : FenceValue(0)
    , CommandListType(Type)
    , Device(Device)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = Type;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	THROW_IF_FAILED(Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&CommandQueue)));
	THROW_IF_FAILED(Device->CreateCommandAllocator(Type, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));
	THROW_IF_FAILED(Device->CreateCommandList(0, Type, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(CommandList.GetAddressOf())));

	FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	CHECK(FenceEvent);

	CommandList->Close();
	IsReset = false;
	THROW_IF_FAILED(Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
}

OCommandQueue::~OCommandQueue()
{
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> OCommandQueue::GetCommandList()
{
	return CommandList;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> OCommandQueue::GetCommandAllocator()
{
	return CommandAllocator;
}

uint64_t OCommandQueue::ExecuteCommandList()
{
	if (!IsReset)
	{
		LOG(Engine, Warning, "Command list has to be reset before closing!");
		return 0;
	}

	IsReset = false;
	CommandList->Close();
	ID3D12CommandList* const commandLists[] = {
		CommandList.Get()
	};

	CommandQueue->ExecuteCommandLists(1, commandLists);
	uint64_t fenceValue = Signal();
	return fenceValue;
}

void OCommandQueue::ExecuteCommandListAndWait()
{
	WaitForFenceValue(ExecuteCommandList());
}

/*
 * Purpose: It's used to indicate a point in time in the command queue and is commonly used for synchronization purposes.
*    The GPU processes commands in the order they are placed in the command queue.
*    By placing a signal command in the command queue,
*    you ensure that the GPU updates the fence's value only after all preceding commands have been completed.
*    This is crucial for synchronization.
*
    If the fence update were done outside the command queue, there would be no guarantee about when it happens relative to other GPU operations.
    It might signal completion before the actual work is done, leading to incorrect program behavior.
 */

uint64_t OCommandQueue::Signal()
{
	uint64_t fenceValueForSignal = ++FenceValue;
	// Put the fence in the command queue, exlicitly telling up to what point we consider those command to get done.
	THROW_IF_FAILED(CommandQueue->Signal(Fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

bool OCommandQueue::IsFenceComplete(uint64_t FenceValue) const
{
	return Fence->GetCompletedValue() >= FenceValue;
}

/*
 *     Purpose: Ensures that the CPU waits for the GPU to reach a certain point in execution.
 */
void OCommandQueue::WaitForFenceValue(uint64_t FenceValue)
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		FenceEvent = ::CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		THROW_IF_FAILED(Fence->SetEventOnCompletion(FenceValue, FenceEvent));
		::WaitForSingleObject(FenceEvent, INFINITE);
		::CloseHandle(FenceEvent);
	}
}

/*
 * This function is a combination of the previous two, signaling a fence and then waiting for its completion.
 * Purpose: Used to ensure that all previously submitted commands on a command queue are completed before the CPU proceeds.
 */
void OCommandQueue::Flush()
{
	WaitForFenceValue(Signal());
}

void OCommandQueue::TryResetCommandList()
{
	if (IsReset)
	{
		LOG(Engine, Warning, "Command list is already reset!");
		return;
	}
	IsReset = true;
	THROW_IF_FAILED(CommandList->Reset(CommandAllocator.Get(), nullptr));
}

Microsoft::WRL::ComPtr<ID3D12Fence> OCommandQueue::GetFence() const
{
	return Fence;
}

void OCommandQueue::SetPipelineState(SPSODescriptionBase* PSOInfo)
{
	if (CurrentPSO == PSOInfo)
	{
		return;
	}

	CurrentPSO = PSOInfo;
	SetResources.clear();
	LOG(Engine, Log, "Setting pipeline state for PSO: {}", TEXT(PSOInfo->Name));
	CommandList->SetPipelineState(PSOInfo->PSO.Get());

	if (CurrentPSO->Type == EPSOType::Graphics)
	{
		CommandList->SetGraphicsRootSignature(PSOInfo->RootSignature->RootSignatureParams.RootSignature.Get());
	}
	else
	{
		CommandList->SetComputeRootSignature(PSOInfo->RootSignature->RootSignatureParams.RootSignature.Get());
	}
}

void OCommandQueue::SetResource(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Resource, SPSODescriptionBase* PSO)
{
	if (CurrentPSO != PSO)
	{
		LOG(Engine, Warning, "Trying to set resource view for a different PSO!")
		SetPipelineState(PSO);
	}

	if (SetResources.contains(Name) && SetResources[Name] == Resource)
	{
		LOG(Engine, Warning, "Resource {} already set!", TEXT(Name));
		return;
	}

	PSO->RootSignature->SetResource(Name, Resource, CommandList.Get());
	SetResources[Name]= Resource;
}

void OCommandQueue::SetResource(const string& Name, D3D12_GPU_DESCRIPTOR_HANDLE Resource, SPSODescriptionBase* PSO)
{
	if (CurrentPSO != PSO)
	{
		LOG(Engine, Error, "Trying to set resource view for a different PSO!")
		SetPipelineState(PSO);
	}

	if (SetResources.contains(Name) && SetResources[Name] == Resource.ptr)
	{
		LOG(Engine, Warning, "Resource {} already set!", TEXT(Name));
		return;
	}

	PSO->RootSignature->SetResource(Name, Resource, CommandList.Get());
	SetResources[Name] = Resource.ptr;
}

void OCommandQueue::ResourceBarrier(ORenderTargetBase* Resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter) const
{
	Utils::ResourceBarrier(CommandList.Get(), Resource->GetResource(), StateBefore, StateAfter);
}

void OCommandQueue::ResourceBarrier(ORenderTargetBase* Resource, D3D12_RESOURCE_STATES StateAfter) const
{
	Utils::ResourceBarrier(CommandList.Get(), Resource->GetResource(), StateAfter);
}

void OCommandQueue::ResourceBarrier(SResourceInfo* Resource, D3D12_RESOURCE_STATES StateAfter) const
{
	Utils::ResourceBarrier(CommandList.Get(), Resource, StateAfter);
}

void OCommandQueue::CopyResourceTo(ORenderTargetBase* Dest, ORenderTargetBase* Src) const
{
	if (Dest == Src)
	{
		LOG(Engine, Error, "Source and destination are the same!");
		return;
	}

	ResourceBarrier(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	ResourceBarrier(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	CommandList->CopyResource(Dest->GetResource()->Resource.Get(), Src->GetResource()->Resource.Get());
	ResourceBarrier(Dest, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

ORenderTargetBase* OCommandQueue::SetRenderTarget(ORenderTargetBase* RenderTarget, uint32_t Subtarget)
{
	if (CurrentRenderTarget && CurrentRenderTarget != RenderTarget)
	{
		CurrentRenderTarget->UnsetRenderTarget(this);
	}

	RenderTarget->PrepareRenderTarget(CommandList.Get(),Subtarget);
	CurrentRenderTarget = RenderTarget;
	return RenderTarget;
}


void OCommandQueue::ResetQueueState()
{
	CurrentRenderTarget->UnsetRenderTarget(this);
	CurrentRenderTarget = nullptr;
	CurrentPSO = nullptr;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> OCommandQueue::GetCommandQueue()
{
	return CommandQueue;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> OCommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	THROW_IF_FAILED(Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(&allocator)));
	return allocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> OCommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> list;
	THROW_IF_FAILED(Device->CreateCommandList(0, CommandListType, Allocator.Get(), nullptr, IID_PPV_ARGS(&list)));
	return list;
}
