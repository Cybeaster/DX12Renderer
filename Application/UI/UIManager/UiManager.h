#pragma once
#include "../Widget.h"
#include "Engine/RenderObject/RenderObject.h"
#include "Events.h"
#include "imgui.h"

class OEngine;
class OUIManager : public OHierarchicalWidgetBase
    , public IRenderObject
{
public:
	void InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap, SRenderObjectDescriptor& OutDescriptor, OEngine* Engine);
	void Draw() override;
	void PostRender(ID3D12GraphicsCommandList* List);
	void Destroy();
	void InitWidgets(OEngine* Engine);
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnKeyboardKeyPressed(KeyEventArgs& Args);
	void OnKeyboardKeyReleased(KeyEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);
	void OnResize(ResizeEventArgs& Args);
	void Update(const UpdateEventArgs& Event) override;
	void KeyMap();

	uint32_t GetNumSRVRequired() const override
	{
		return 10;
	}

	void PostInputUpdate();

	bool IsInFocus() override;
	wstring GetName() override
	{
		return Name;
	}
private:
	bool bIsInFocus = false;
	ImGuiIO* IO = nullptr;
	wstring Name = L"UI Manager";
	float MangerWidth = 400;
	float MangerHeight = 800;
};
