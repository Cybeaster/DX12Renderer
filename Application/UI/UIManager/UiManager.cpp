#include "UiManager.h"

#include "Engine/Engine.h"
#include "UI/Effects/FogWidget.h"
#include "UI/Effects/Light/LightWidget.h"
#include "UI/Engine/Camera.h"
#include "UI/Filters/FilterManager.h"
#include "UI/Geometry/GeometryManager.h"
#include "UI/Material/MaterialManager/MaterialManager.h"
#include "UI/Material/TextureManager/TextureManager.h"
#include "Window/Window.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

void OUIManager::InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap, SRenderObjectDescriptor& OutDescriptor, OEngine* Engine)
{

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	IO = &ImGui::GetIO();
	IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	KeyMap();

	ImGui_ImplWin32_Init(Hwnd);
	auto [CPUHandle, GPUHandle, Index] = OutDescriptor.SRVHandle.Offset(GetNumSRVRequired());
	ImGui_ImplDX12_Init(
	    Device,
	    NumFramesInLight,
	    SRenderConstants::BackBufferFormat,
	    SRVDescriptorHeap,
	    CPUHandle,
	    GPUHandle);

	ImGui::StyleColorsDark();
	InitWidgets(Engine);
}

void OUIManager::Draw()
{
	PostInputUpdate();

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(MangerWidth, MangerHeight));
	ImGui::Begin("Main Menu");

	OHierarchicalWidgetBase::Draw();

	ImGui::End();
	ImGui::ShowDemoWindow();
	ImGui::Render();
}

void OUIManager::PostRender(ID3D12GraphicsCommandList* List)
{
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), List);
}

void OUIManager::Destroy()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();
}

void OUIManager::InitWidgets(OEngine* Engine)
{
	MakeWidget<OFilterManagerWidget>(Engine);
	MakeWidget<OFogWidget>(Engine);
	MakeWidget<OLightWidget>(Engine);
	MakeWidget<OCameraWidget>(Engine->GetWindow()->GetCamera());
	MakeWidget<OGeometryManagerWidget>(Engine, &Engine->GetRenderLayers());
	MakeWidget<OMaterialManagerWidget>(Engine->GetMaterialManager());
	MakeWidget<OTextureManagerWidget>(Engine->GetTextureManager());
}

void OUIManager::OnMouseButtonPressed(MouseButtonEventArgs& Args)
{
	switch (Args.Button)
	{
	case MouseButtonEventArgs::None:
		break;
	case MouseButtonEventArgs::Left:
		IO->MouseDown[0] = true;
		break;
	case MouseButtonEventArgs::Right:
		IO->MouseDown[1] = true;
		break;
	case MouseButtonEventArgs::Middel:
		IO->MouseDown[2] = true;
		break;
	}
}

void OUIManager::OnMouseButtonReleased(MouseButtonEventArgs& Args)
{
	switch (Args.Button)
	{
	case MouseButtonEventArgs::None:
		break;
	case MouseButtonEventArgs::Left:
		IO->MouseDown[0] = false;
		break;
	case MouseButtonEventArgs::Right:
		IO->MouseDown[1] = false;
		break;
	case MouseButtonEventArgs::Middel:
		IO->MouseDown[2] = false;
		break;
	}
}

void OUIManager::OnKeyboardKeyPressed(KeyEventArgs& Args)
{
	IO->KeysDown[Args.Key] = true;
	IO->AddInputCharacter(Args.Char);
}

void OUIManager::OnKeyboardKeyReleased(KeyEventArgs& Args)
{
	IO->KeysDown[Args.Key] = false;
}

void OUIManager::OnMouseWheel(MouseWheelEventArgs& Args)
{
	IO->MouseWheel += Args.WheelDelta;
}

void OUIManager::OnResize(ResizeEventArgs& Args)
{
	if (IO)
	{
		IO->DisplaySize = ImVec2(static_cast<float>(Args.Width), static_cast<float>(Args.Height));
	}
}
void OUIManager::Update(const UpdateEventArgs& Event)
{
	OHierarchicalWidgetBase::Update();
}

void OUIManager::KeyMap()
{
	ImGuiIO& io = ImGui::GetIO();

	// Arrow keys
	io.KeyMap[ImGuiKey_LeftArrow] = KeyCode::Left;
	io.KeyMap[ImGuiKey_RightArrow] = KeyCode::Right;
	io.KeyMap[ImGuiKey_UpArrow] = KeyCode::Up;
	io.KeyMap[ImGuiKey_DownArrow] = KeyCode::Down;

	// Navigation keys
	io.KeyMap[ImGuiKey_PageUp] = KeyCode::PageUp;
	io.KeyMap[ImGuiKey_PageDown] = KeyCode::PageDown;
	io.KeyMap[ImGuiKey_Home] = KeyCode::Home;
	io.KeyMap[ImGuiKey_End] = KeyCode::End;
	io.KeyMap[ImGuiKey_Insert] = KeyCode::Insert;
	io.KeyMap[ImGuiKey_Delete] = KeyCode::Delete;
	io.KeyMap[ImGuiKey_Backspace] = KeyCode::Back;
	io.KeyMap[ImGuiKey_Space] = KeyCode::Space;
	io.KeyMap[ImGuiKey_Enter] = KeyCode::Enter;
	io.KeyMap[ImGuiKey_Escape] = KeyCode::Escape;
	io.KeyMap[ImGuiKey_Tab] = KeyCode::Tab;

	// Function keys
	io.KeyMap[ImGuiKey_F1] = KeyCode::F1;
	io.KeyMap[ImGuiKey_F2] = KeyCode::F2;
	io.KeyMap[ImGuiKey_F3] = KeyCode::F3;
	io.KeyMap[ImGuiKey_F4] = KeyCode::F4;
	io.KeyMap[ImGuiKey_F5] = KeyCode::F5;
	io.KeyMap[ImGuiKey_F6] = KeyCode::F6;
	io.KeyMap[ImGuiKey_F7] = KeyCode::F7;
	io.KeyMap[ImGuiKey_F8] = KeyCode::F8;
	io.KeyMap[ImGuiKey_F9] = KeyCode::F9;
	io.KeyMap[ImGuiKey_F10] = KeyCode::F10;
	io.KeyMap[ImGuiKey_F11] = KeyCode::F11;
	io.KeyMap[ImGuiKey_F12] = KeyCode::F12;
	// Add additional function keys if needed

	// Alpha-numeric keys (A-Z, 0-9)
	// ImGui does not differentiate between upper and lower case keys for mapping.
	for (int i = 0; i < 26; ++i)
	{ // Map A-Z
		io.KeyMap[ImGuiKey_A + i] = KeyCode::A + i;
	}
	for (int i = 0; i < 10; ++i)
	{ // Map 0-9
		io.KeyMap[ImGuiKey_0 + i] = KeyCode::D0 + i;
	}
}

void OUIManager::PostInputUpdate()
{
	bIsInFocus = ImGui::IsAnyItemFocused() || ImGui::IsAnyItemActive() || ImGui::IsItemClicked();
}
bool OUIManager::IsInFocus()
{
	return bIsInFocus;
}
