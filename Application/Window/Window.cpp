
#include "Window.h"

#include "Application.h"
#include "Camera/Camera.h"
#include "Exception.h"
#include "Logger.h"

#pragma optimize("", off)
using namespace Microsoft::WRL;

OWindow::OWindow(HWND hWnd, const SWindowInfo& _WindowInfo)
    : ORenderTargetBase(WindowInfo.ClientWidth, WindowInfo.ClientHeight), Hwnd(hWnd), WindowInfo{ _WindowInfo }
{
	Name = WindowInfo.Name;
}

void OWindow::InitRenderObject()
{
	Camera = make_shared<OCamera>(this);
	SetCameraLens();
	BuildResource();
}

void OWindow::BuildResource()
{
	const auto engine = OEngine::Get();
	auto device = engine->GetDevice();
	SwapChain = CreateSwapChain();
	TotalRTVDescNum = OEngine::Get()->GetDesiredCountOfRTVs();
	TotalDSVDescNum = OEngine::Get()->GetDesiredCountOfDSVs();

	RTVDescriptorSize = engine->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	RTVHeap = engine->CreateDescriptorHeap(TotalRTVDescNum, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DSVHeap = engine->CreateDescriptorHeap(TotalDSVDescNum, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	LOG(Render, Log, "RTV and DSV heaps created. {} RTVs and {} DSVs", TotalRTVDescNum, TotalDSVDescNum);
	UpdateRenderTargetViews();
	ResizeDepthBuffer();
}

void OWindow::BuildDescriptors(IDescriptor* Descriptor)
{
	/*auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}
	RTVHandle = descriptor->RTVHandle.Offset(2);
	DSVHandle = descriptor->DSVHandle.Offset(1);
	UpdateRenderTargetViews();
	ResizeDepthBuffer();*/
	BuildDescriptors();
}

const wstring& OWindow::GetName() const
{
	return WindowInfo.Name;
}

uint32_t OWindow::GetWidth() const
{
	return WindowInfo.ClientWidth;
}

uint32_t OWindow::GetHeight() const
{
	return WindowInfo.ClientHeight;
}

UINT OWindow::GetCurrentBackBufferIndex() const
{
	return CurrentBackBufferIndex;
}

UINT OWindow::Present()
{
	THROW_IF_FAILED(SwapChain->Present(0, 0));
	CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	return CurrentBackBufferIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE OWindow::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart(),
	                                     CurrentBackBufferIndex,
	                                     RTVDescriptorSize);
}

SResourceInfo* OWindow::GetCurrentBackBuffer()
{
	return &BackBuffers[CurrentBackBufferIndex];
}

SResourceInfo* OWindow::GetCurrentDepthStencilBuffer()
{
	return &DepthBuffer;
}

bool OWindow::IsVSync() const
{
	return WindowInfo.VSync;
}

void OWindow::SetVSync(bool vSync)
{
	WindowInfo.VSync = vSync;
}

void OWindow::ToggleVSync()
{
	SetVSync(!WindowInfo.VSync);
}

float OWindow::GetFoV()
{
	return WindowInfo.FoV;
}

void OWindow::SetFoV(float _FoV)
{
	WindowInfo.FoV = _FoV;
}

float OWindow::GetAspectRatio() const
{
	return static_cast<float>(WindowInfo.ClientWidth) / static_cast<float>(WindowInfo.ClientHeight);
}

bool OWindow::IsFullScreen() const
{
	return WindowInfo.Fullscreen;
}

void OWindow::SetFullscreen(bool _Fullscreen)
{
	if (WindowInfo.Fullscreen != _Fullscreen)
	{
		WindowInfo.Fullscreen = _Fullscreen;
		if (WindowInfo.Fullscreen)
		{
			// Store the current window dimensions so they can be restored
			// when switching out of fullscreen state.
			::GetWindowRect(Hwnd, &WindowRect);

			// Set the window style to a borderless window so the client area fills
			// the entire screen.
			constexpr UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

			::SetWindowLongW(Hwnd, GWL_STYLE, windowStyle);

			// Query the name of the nearest display device for the window.
			// This is required to set the fullscreen dimensions of the window
			// when using a multi-monitor setup.
			const HMONITOR hMonitor = ::MonitorFromWindow(Hwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorInfo);

			::SetWindowPos(Hwnd, HWND_TOPMOST, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(Hwnd, SW_MAXIMIZE);
		}
		else
		{
			// Restore all the window decorators.
			::SetWindowLong(Hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(Hwnd, HWND_NOTOPMOST, WindowRect.left, WindowRect.top, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(Hwnd, SW_NORMAL);
		}
	}
}

void OWindow::ToggleFullscreen()
{
	SetFullscreen(!WindowInfo.Fullscreen);
}

void OWindow::Show()
{
	::ShowWindow(Hwnd, SW_SHOW);
}

void OWindow::Hide()
{
	::ShowWindow(Hwnd, SW_HIDE);
}

void OWindow::RegsterWindow()
{
	Show();
	UpdateWindow(Hwnd);
}

void OWindow::Destroy()
{
	if (const auto engine = OEngine::Get())
	{
		engine->OnWindowDestroyed();
	}

	if (Hwnd && !::DestroyWindow(Hwnd))
	{
		LOG(Engine, Error, "Failed to destroy window.");
	}
}

void OWindow::OnRender(const UpdateEventArgs& Event)
{
}

void OWindow::Update(const UpdateEventArgs& Event)
{
	if (Event.IsUIInfocus)
	{
		return;
	}

	const auto time = Event.Timer.GetDeltaTime();
	if (IsKeyPressed(KeyCode::W))
	{
		Camera->MoveToTarget(time);
	}
	if (IsKeyPressed(KeyCode::S))
	{
		Camera->MoveToTarget(-time);
	}
	if (IsKeyPressed(KeyCode::A))
	{
		Camera->Strafe(-time);
	}
	if (IsKeyPressed(KeyCode::D))
	{
		Camera->Strafe(time);
	}
	Camera->UpdateViewMatrix();
}

void OWindow::OnKeyPressed(KeyEventArgs& Event)
{
	LOG(Input, Log, "Key Pressed: {}, new camera position: {}", TEXT(Event.Key), TEXT(Camera->GetPosition3f()));
}

void OWindow::OnKeyReleased(KeyEventArgs& Event)
{
}

void OWindow::OnMouseMoved(MouseMotionEventArgs& Event)
{
	if (Event.IsUIInfocus)
	{
		return;
	}

	if (Event.RightButton)
	{
		const float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(Event.X - LastMouseXPos));
		const float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(Event.Y - LastMouseYPos));
		Camera->Pitch(dy);
		Camera->RotateY(dx);
	}

	if (Event.LeftButton && (OEngine::Get()->GetTime() - LeftButtonPressedTime) > CameraLeftMoveTime)
	{
		const float dx = 0.1 * static_cast<float>(Event.X - LastMouseXPos);
		Camera->MoveToTarget(dx * OEngine::Get()->GetDeltaTime());
	}

	LastMouseXPos = Event.X;
	LastMouseYPos = Event.Y;
}

void OWindow::OnMouseButtonPressed(MouseButtonEventArgs& Event)
{
	if (Event.IsUIInfocus)
	{
		return;
	}

	LastMouseXPos = Event.X;
	LastMouseYPos = Event.Y;

	if (Event.LeftButton)
	{
		LeftButtonPressedTime = OEngine::Get()->GetTime();
	}

	SetCapture(Hwnd);
}

void OWindow::OnMouseButtonReleased(MouseButtonEventArgs& Event)
{
	if (Event.Button == MouseButtonEventArgs::EMouseButton::Left)
	{
		if (!HasCapturedLeftMouseButton())
		{
			OEngine::Get()->Pick(Event.X, Event.Y);
		}
		LeftButtonPressedTime = 0;
	}

	ReleaseCapture();
}

void OWindow::OnMouseWheel(MouseWheelEventArgs& Event)
{
	Camera->UpdateCameraSpeed(Event.WheelDelta);
}

void OWindow::MoveToNextFrame()
{
	CurrentBackBufferIndex = (CurrentBackBufferIndex + 1) % SRenderConstants::RenderBuffersCount;
}

const ComPtr<IDXGISwapChain4>& OWindow::GetSwapChain()
{
	return SwapChain;
}

void OWindow::TransitionResource(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState)
{
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), BeforeState, AfterState);
	CommandList->ResourceBarrier(1, &barrier);
}

void OWindow::ClearRTV(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor)
{
	CommandList->ClearRenderTargetView(RTV, ClearColor, 0, nullptr);
}

void OWindow::ClearDepth(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth)
{
	CommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, Depth, 0, 0, nullptr);
}

void OWindow::OnUpdateWindowSize(ResizeEventArgs& Event)
{
	WindowInfo.ClientHeight = Event.Height;
	WindowInfo.ClientWidth = Event.Width;
}

void OWindow::OnResize(ResizeEventArgs& Event)
{
	SetCameraLens();
	ResetBuffers();
}

void OWindow::ResetBuffers()
{
	if (SwapChain)
	{
		OEngine::Get()->GetCommandQueue()->TryResetCommandList();
		for (int i = 0; i < SRenderConstants::RenderBuffersCount; ++i)
		{
			// Flush any GPU commands that might be referencing the back buffers
			BackBuffers[i].Resource.Reset();
		}

		THROW_IF_FAILED(SwapChain->ResizeBuffers(SRenderConstants::RenderBuffersCount, WindowInfo.ClientWidth, WindowInfo.ClientHeight, SRenderConstants::BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
		CurrentBackBufferIndex = 0;

		UpdateRenderTargetViews();
		ResizeDepthBuffer();
		OEngine::Get()->GetCommandQueue()->ExecuteCommandListAndWait();
		Viewport = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<float>(WindowInfo.ClientWidth), static_cast<float>(WindowInfo.ClientHeight), 0.0f, 1.0f);
		ScissorRect = CD3DX12_RECT(0, 0, WindowInfo.ClientWidth, WindowInfo.ClientHeight);
	}
}

void OWindow::SetCameraLens()
{
	if (Camera)
	{
		Camera->SetLens(0.25 * DirectX::XM_PI, GetAspectRatio(), 1.0f, 1000.0f);
	}
}
uint32_t OWindow::GetNumDSVRequired() const
{
	return 1;
}

uint32_t OWindow::GetNumRTVRequired() const
{
	return 3;
}
SDescriptorPair OWindow::GetRTV(uint32_t SubtargetIdx) const
{
	SDescriptorPair pair;
	pair.CPUHandle = CurrentBackBufferView();

	return pair; // TODO - return the correct RTV
}
SDescriptorPair OWindow::GetDSV(uint32_t SubtargetIdx) const
{
	SDescriptorPair pair;
	pair.CPUHandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
	return pair; //TODO - return the correct DSV
}


SResourceInfo* OWindow::GetResource()
{
	return GetCurrentBackBuffer();
}

float OWindow::GetLastXMousePos() const
{
	return LastMouseXPos;
}

float OWindow::GetLastYMousePos() const
{
	return LastMouseYPos;
}

shared_ptr<OCamera> OWindow::GetCamera()
{
	return Camera;
}

uint32_t OWindow::GetDSVDescNum() const
{
	return TotalDSVDescNum;
}

uint32_t OWindow::GetRTVDescNum() const
{
	return TotalRTVDescNum;
}

bool OWindow::HasCapturedLeftMouseButton() const
{
	return LeftButtonPressedTime > 0 && OEngine::Get()->GetTime() - LeftButtonPressedTime > CameraLeftMoveTime;
}

ComPtr<IDXGISwapChain4> OWindow::CreateSwapChain()
{
	const auto engine = OEngine::Get();
	ComPtr<IDXGISwapChain4> swapChain4;
	UINT msaaQuality;
	const bool msaaState = engine->GetMSAAState(msaaQuality);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = WindowInfo.ClientWidth;
	swapChainDesc.Height = WindowInfo.ClientHeight;
	swapChainDesc.Format = SRenderConstants::BackBufferFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = msaaState ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = msaaState ? msaaQuality - 1 : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SRenderConstants::RenderBuffersCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	const auto commandQueue = engine->GetCommandQueue()->GetCommandQueue();

	ComPtr<IDXGISwapChain1> swapChain1;
	THROW_IF_FAILED(engine->GetFactory()->CreateSwapChainForHwnd(
	    commandQueue.Get(),
	    Hwnd,
	    &swapChainDesc,
	    nullptr,
	    nullptr,
	    &swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	THROW_IF_FAILED(engine->GetFactory()->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_ALT_ENTER));
	THROW_IF_FAILED(swapChain1.As(&swapChain4));

	CurrentBackBufferIndex = swapChain4->GetCurrentBackBufferIndex();

	return swapChain4;
}

void OWindow::UpdateRenderTargetViews()
{
	const auto device = OEngine::Get()->GetDevice();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SRenderConstants::RenderBuffersCount; i++)
	{
		THROW_IF_FAILED(SwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffers[i].Resource)));
		device->CreateRenderTargetView(BackBuffers[i].Resource.Get(), nullptr, rtvHandle);
		BackBuffers[i].CurrentState = D3D12_RESOURCE_STATE_PRESENT;
		BackBuffers[i].Context = this;
		BackBuffers[i].Resource->SetName(Name.c_str());
		rtvHandle.Offset(RTVDescriptorSize);
	}
}

void OWindow::ResizeDepthBuffer()
{
	// Flush any GPU commands that might be referencing the depth buffer.
	const auto engine = OEngine::Get();
	engine->GetCommandQueue()->TryResetCommandList();
	auto list = engine->GetCommandQueue()->GetCommandList();
	UINT quality;
	auto mxaaEnabled = engine->GetMSAAState(quality);
	DepthBuffer.Resource.Reset();
	const auto device = engine->GetDevice();

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = GetWidth();
	depthStencilDesc.Height = GetHeight();
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = mxaaEnabled ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = mxaaEnabled ? (quality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = SRenderConstants::DepthBufferFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	DepthBuffer = Utils::CreateResource(this,
	                                    device.Get(),
	                                    D3D12_HEAP_TYPE_DEFAULT,
	                                    depthStencilDesc,
	                                    D3D12_RESOURCE_STATE_DEPTH_WRITE,
	                                    engine->GetCommandQueue()->GetCommandList().Get(),
	                                    &optClear);

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = SRenderConstants::DepthBufferFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(DepthBuffer.Resource.Get(), &dsvDesc, GetDepthStensilView());
	engine->GetCommandQueue()->ExecuteCommandListAndWait();
}

void OWindow::BuildDescriptors()
{
	ResetBuffers();
}

HWND OWindow::GetHWND() const
{
	return Hwnd;
}

ComPtr<ID3D12DescriptorHeap> OWindow::GetDSVDescriptorHeap() const
{
	return DSVHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE OWindow::GetDepthStensilView() const
{
	return DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

#pragma optimize("", on)
