#pragma once
#include "DirectX/InstanceData.h"
#include "Engine/RenderObject/RenderObject.h"

class OWaterRenderObject : public ORenderObjectBase
{
public:
	void InitRenderObject() override;
	void Update(const UpdateEventArgs& Event) override;

private:
	vector<SInstanceData> InstanceData;
};



