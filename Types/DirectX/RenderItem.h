#pragma once
#include "../../Materials/Material.h"
#include "..\..\Utils\Math.h"
#include "DXHelper.h"
#include "RenderConstants.h"

struct SRenderItem
{
	SRenderItem() = default;
	void UpdateWorldMatrix(const DirectX::XMMATRIX& WorldMatrix)
	{
		DirectX::XMStoreFloat4x4(&World, WorldMatrix);
		NumFramesDirty = SRenderConstants::NumFrameResources;
	}
	// World matrix of the shape that describes the object’s local space
	// relative to the world space, which defines the position,
	// orientation, and scale of the object in the world.
	DirectX::XMFLOAT4X4 World = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Utils::Math::Identity4x4();

	DirectX::XMFLOAT2 DisplacementMapTexelSize = { 1.0f, 1.0f };
	float GridSpatialStep = 1.0f;

	// Dirty flag indicating the object data has changed and we need
	// to update the constant buffer. Because we have an object
	// cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource. Thus, when we modify obect data we
	// should set
	// NumFramesDirty = NumFrameResources so that each frame resource
	// gets the update.

	uint32_t NumFramesDirty = SRenderConstants::NumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjectCBIndex = -1;

	SMaterial* Material = nullptr;
	SMeshGeometry* Geometry = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};
