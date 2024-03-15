
#include "CubeMapTest.h"

#include "Application.h"
#include "EngineHelper.h"
#include "Geometry/GPUWave/GpuWave.h"
#include "Window/Window.h"

bool OCubeMapTest::Initialize()
{
	OEngine::Get()->BuildCubeRenderTarget({ 0, 2, 0 });
	Water = OEngine::Get()->BuildRenderObject<OWaterRenderObject>();
	BuildRenderItems();
	return true;
}

void OCubeMapTest::OnUpdate(const UpdateEventArgs& Event)
{
	OTest::OnUpdate(Event);
	AnimateSkull(Event);
}

void OCubeMapTest::AnimateSkull(const UpdateEventArgs& Event)
{
	using namespace DirectX;
	XMMATRIX skullScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	XMMATRIX skullOffset = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	XMMATRIX skullLocalRotate = XMMatrixRotationY(2.0f * Event.Timer.GetTime());
	XMMATRIX skullGlobalRotate = XMMatrixRotationY(0.5f * Event.Timer.GetTime());
	XMStoreFloat4x4(&SkullRitem->World, skullScale * skullLocalRotate * skullOffset * skullGlobalRotate);
}

void OCubeMapTest::BuildRenderItems()
{
	auto& sky = CreateSphereRenderItem(SRenderLayer::Sky,
	                                   "Sky",
	                                   0.5f,
	                                   20,
	                                   20,
	                                   FindMaterial("GrassSky"));
	Scale(sky[0].World, { 5000.0f, 5000.0f, 5000.0f });

	auto& skull = CreateRenderItem(SRenderLayer::Opaque,
	                               "Skull",
	                               WStringToUTF8(OApplication::Get()->GetResourcePath(L"Resources/Models/skull.txt")),
	                               EParserType::Custom,
	                               ETextureMapType::Spherical,
	                               SRenderItemParams{ FindMaterial("White") });
	Scale(skull[0].World, { 0.5f, 0.5f, 0.5f });
	SkullRitem = &skull[0];

	auto& box = CreateBoxRenderItem(SRenderLayer::Opaque,
	                                "Box",
	                                1.0f,
	                                1.0f,
	                                1.0f,
	                                3,
	                                FindMaterial("Tile"));
	box[0].World = Scale(Translate(box[0].World, { 2.0f, 1.0f, 2.0f }), { 1.0f, 2.0f, 1.0f });

	auto& globe = CreateSphereRenderItem(SRenderLayer::OpaqueDynamicReflections,
	                                     "Globe",
	                                     0.5f,
	                                     20,
	                                     20,
	                                     FindMaterial("Mirror"));

	Scale(globe[0].World, { 3, 3, 3 });
	Translate(globe[0].World, { 0, 3, 0 });

	auto& grid
	    = CreateGridRenderItem(SRenderLayer::Opaque,
	                           "Grid",
	                           20.0f,
	                           30.0f,
	                           60,
	                           40,
	                           FindMaterial("Tile"));
	Scale(grid[0].TexTransform, { 8, 8, 1.0f });

	auto& cylinder = CreateCylinderRenderItem(SRenderLayer::Opaque,
	                                          "Cylinder",
	                                          0.5f,
	                                          0.5f,
	                                          3.0f,
	                                          20,
	                                          20,
	                                          { FindMaterial("Bricks"), 10 });

	auto& spheres = CreateSphereRenderItem(SRenderLayer::Opaque,
	                                       "Spheres",
	                                       0.5f,
	                                       20,
	                                       20,
	                                       { FindMaterial("Mirror"), 10 });

	using namespace DirectX;
	XMMATRIX brickTexTransform = XMMatrixScaling(1.5f, 2.0f, 1.0f);
	for (size_t it = 0; it < 10; it += 2)
	{
		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + it * 5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + it * 5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + it * 5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + it * 5.0f);

		auto& leftSphere = spheres[it];
		auto& rightSphere = spheres[it + 1];

		auto& leftCylinder = cylinder[it];
		auto& rightCylinder = cylinder[it + 1];

		Put(leftSphere.World, leftSphereWorld);
		Put(rightSphere.World, rightSphereWorld);
		Put(rightCylinder.TexTransform, brickTexTransform);
		Put(leftCylinder.TexTransform, brickTexTransform);
		Put(leftCylinder.World, leftCylWorld);
		Put(rightCylinder.World, rightCylWorld);
	}

	SRenderItemParams params;
	params.NumberOfInstances = 1;
	params.bFrustrumCoolingEnabled = false;
	params.Pickable = false;
	params.MaterialParams.Material = FindMaterial("Water01");


}