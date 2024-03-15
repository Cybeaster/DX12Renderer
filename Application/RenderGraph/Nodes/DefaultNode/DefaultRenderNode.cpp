
#include "DefaultRenderNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"

void ODefaultRenderNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResources;

	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart(), PSO);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(), PSO);
}

void ODefaultRenderNode::Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue,
                                    ORenderGraph* OtherParentGraph, SPSODescriptionBase* OtherPSO)
{
	ORenderNode::Initialize(OtherNodeInfo, OtherCommandQueue, OtherParentGraph, OtherPSO);
}

ORenderTargetBase* ODefaultRenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(PSO, GetNodeInfo().RenderLayer);
	return RenderTarget;
}