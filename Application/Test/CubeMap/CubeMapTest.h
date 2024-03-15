#pragma once
#include "Geometry/Water/Water.h"
#include "Test/Test.h"

class OGPUWave;
class OCubeMapTest : public OTest
{
public:
	OCubeMapTest(OWindow* Window)
	    : OTest(Window) {}

	bool Initialize() override;
	void OnUpdate(const UpdateEventArgs& Event) override;
	void AnimateSkull(const UpdateEventArgs& Event);
	void BuildRenderItems();

private:
	OWaterRenderObject* Water = nullptr;

	SInstanceData* SkullRitem = nullptr;
};
