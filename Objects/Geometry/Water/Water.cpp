//
// Created by Cybea on 14/03/2024.
//

#include "Water.h"

#include "EngineHelper.h"
void OWaterRenderObject::InitRenderObject()
{
	SRenderItemParams params;
	params.NumberOfInstances = 1;
	params.bFrustrumCoolingEnabled = false;
	params.Pickable = false;
	params.MaterialParams.Material = FindMaterial("Water01");
	InstanceData = CreateGridRenderItem(SRenderLayer::Water,
	                                    "Water",
	                                    160,
	                                    160,
	                                    300,
	                                    300,
	                                    params);
}
void OWaterRenderObject::Update(const UpdateEventArgs& Event)
{
	for (auto& inst : InstanceData)
	{
		auto timer = Event.Timer;
		const auto mat = FindMaterial(inst.MaterialIndex);
		float& tu = mat->MatTransform(3, 0);
		float& tv = mat->MatTransform(3, 1);
		tu += 0.1f * timer.GetDeltaTime();
		tv += 0.02f * timer.GetDeltaTime();

		if (tu >= 1.0)
		{
			tu -= 1.0f;
		}

		if (tv >= 1.0)
		{
			tv -= 1.0f;
		}

		mat->MatTransform(3, 0) = tu;
		mat->MatTransform(3, 1) = tv;
		mat->NumFramesDirty = SRenderConstants::NumFrameResources;
	}
}