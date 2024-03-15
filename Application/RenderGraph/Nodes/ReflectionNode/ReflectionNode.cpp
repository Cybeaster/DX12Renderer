
#include "ReflectionNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"
#include "Window/Window.h"
ORenderTargetBase* OReflectionNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto cube = OEngine::Get()->GetCubeRenderTarget();
	auto cmdList = CommandQueue->GetCommandList();
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(),PSO);

	cube->SetViewport(CommandQueue->GetCommandList().Get());
	for (size_t i = 0; i < cube->GetNumRTVRequired(); i++)
	{
		CommandQueue->SetPipelineState(PSO);
		CommandQueue->SetRenderTarget(cube, i);
	  	CommandQueue->SetResource("cbPass", cube->GetPassConstantAddresss(i), PSO);
		OEngine::Get()->DrawRenderItems(PSO, SRenderLayer::Opaque);
		//OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOType::WavesRender)->RootSignature.get(), SRenderLayer::Waves);
	   OEngine::Get()->DrawRenderItems(PSO, SRenderLayer::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cube->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	OEngine::Get()->SetWindowViewport(); //TODO remove this to other place

	CommandQueue->SetResource("cbPass", OEngine::Get()->CurrentFrameResources->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gCubeMap", cube->GetSRVHandle().GPUHandle, PSO);

	CommandQueue->SetRenderTarget(RenderTarget);

	OEngine::Get()->DrawRenderItems(PSO, GetNodeInfo().RenderLayer);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(),PSO);
	return RenderTarget;
}