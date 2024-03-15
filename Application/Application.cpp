#include "Application.h"

#include "Camera/Camera.h"
#include "Engine/Engine.h"
#include "Exception.h"

#include <DirectXMath.h>
#include <Windowsx.h>

using namespace Microsoft::WRL;
OApplication* OApplication::Get()
{
	if (Application == nullptr)
	{
		Application = new OApplication();
	}
	return Application;
}

void OApplication::Destory()
{
	delete Application;
	Application = nullptr;
}

OWindow* OApplication::CreateWindow() const
{
	RECT windowRect = {
		0,
		0,
		static_cast<LONG>(DefaultWindowInfo.ClientWidth),
		static_cast<LONG>(DefaultWindowInfo.ClientHeight)
	};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindowW(WindowClassName,
	                          DefaultWindowInfo.Name.c_str(),
	                          WS_OVERLAPPEDWINDOW,
	                          CW_USEDEFAULT,
	                          CW_USEDEFAULT,
	                          windowRect.right - windowRect.left,
	                          windowRect.bottom - windowRect.top,
	                          nullptr,
	                          nullptr,
	                          AppInstance,
	                          nullptr);
	if (!hWnd)
	{
		MessageBoxA(NULL, "Failed to create window.", "Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}
	auto window = new OWindow(hWnd, DefaultWindowInfo);
	return window;
}

void OApplication::Quit(int ExitCode)
{
	PostQuitMessage(ExitCode);
}

void OApplication::InitApplication(HINSTANCE hInstance)
{
	AppInstance = hInstance;
	ConfigReader = make_unique<OConfigReader>(RootDirPath.GetPath() + "/Resources/Config/Config.json");

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	InitWindowClass();
	Engine = OEngine::Get();
	Engine->Initialize();
}

HINSTANCE OApplication::GetAppInstance() const
{
	return AppInstance;
}

OApplication::OApplication()
{
	CurrentPath.SetPath(std::filesystem::current_path());
	RootDirPath.SetPath(std::filesystem::current_path().parent_path());
}

void OApplication::InitWindowClass() const
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AppInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = OApplication::WindowClassName;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"Unable to register the window class.", L"Error", MB_OK | MB_ICONERROR);
	}
}

void OApplication::CalculateFrameStats()
{
	static int frameCount = 0;
	static float timeElapsed = 0.f;

	frameCount++;
	if (Timer.GetTime() - timeElapsed >= 1.0f)
	{
		float fps = static_cast<float>(frameCount);
		float mspf = 1000.f / fps;

		const std::wstring windowText = SLogUtils::Format(L"FPS: {} MSPF: {}", fps, mspf);
		SetWindowTextW(Engine->GetWindow()->GetHWND(), windowText.c_str());
		frameCount = 0;
		timeElapsed += 1.0f;
	}
}

// Convert the message ID into a MouseButton ID
static MouseButtonEventArgs::EMouseButton DecodeMouseButton(UINT messageID)
{
	MouseButtonEventArgs::EMouseButton mouseButton = MouseButtonEventArgs::None;
	switch (messageID)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	{
		mouseButton = MouseButtonEventArgs::Left;
	}
	break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	{
		mouseButton = MouseButtonEventArgs::Right;
	}
	break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	{
		mouseButton = MouseButtonEventArgs::Middel;
	}
	break;
	}

	return mouseButton;
}

LRESULT CALLBACK OApplication::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto app = Get();
	auto engine = OEngine::Get();
	if (engine)
	{
		bool lButton = (wParam & MK_LBUTTON) != 0;
		bool rButton = (wParam & MK_RBUTTON) != 0;
		bool mButton = (wParam & MK_MBUTTON) != 0;
		bool shift = (wParam & MK_SHIFT) != 0;
		bool control = (wParam & MK_CONTROL) != 0;

		bool asyncShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
		bool asyncControl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		int width = static_cast<short>(LOWORD(lParam));
		int height = static_cast<short>(HIWORD(lParam));

		switch (message)
		{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				app->SetAppPaused(true);
				app->Timer.Stop();
			}
			else
			{
				app->SetAppPaused(false);
				app->Timer.Start();
			}
			break;
		}
		break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			MSG charMsg;
			// Get the Unicode character (UTF-16)
			unsigned int c = 0;
			// For printable characters, the next message will be WM_CHAR.
			// This message contains the character code we need to send the KeyPressed event.
			// Inspired by the SDL 1.2 implementation.
			if (PeekMessage(&charMsg, hwnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
			{
				GetMessage(&charMsg, hwnd, 0, 0);
				c = static_cast<unsigned int>(charMsg.wParam);
			}
			bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			KeyCode::Key key = static_cast<KeyCode::Key>(wParam);
			KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Pressed, asyncShift, asyncControl, alt, hwnd);
			engine->OnKeyPressed(keyEventArgs);
		}
		break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			KeyCode::Key key = static_cast<KeyCode::Key>(wParam);
			unsigned int c = 0;
			unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

			// Determine which key was released by converting the key code and the scan code
			// to a printable character (if possible).
			// Inspired by the SDL 1.2 implementation.
			unsigned char keyboardState[256];
			GetKeyboardState(keyboardState);

			wchar_t translatedCharacters[4];
			if (int result = ToUnicodeEx(static_cast<UINT>(wParam), scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
			{
				c = translatedCharacters[0];
			}

			KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Released, asyncControl, asyncShift, alt, hwnd);
			engine->OnKeyReleased(keyEventArgs);
		}
		break;
		// The default window procedure will play a system notification sound
		// when pressing the Alt+Enter keyboard combination if this message is
		// not handled.
		case WM_SYSCHAR:
			break;
		case WM_MOUSEMOVE:
		{
			MouseMotionEventArgs mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y, hwnd);
			engine->OnMouseMoved(mouseMotionEventArgs);
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Pressed, lButton, mButton, rButton, control, shift, x, y, hwnd);
			engine->OnMouseButtonPressed(mouseButtonEventArgs);
		}
		break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Released, lButton, mButton, rButton, control, shift, x, y, hwnd);
			engine->OnMouseButtonReleased(mouseButtonEventArgs);
		}
		break;
		case WM_MOUSEWHEEL:
		{
			// The distance the mouse wheel is rotated.
			// A positive value indicates the wheel was rotated to the right.
			// A negative value indicates the wheel was rotated to the left.
			float zDelta = static_cast<int>(static_cast<short>(HIWORD(wParam))) / static_cast<float>(WHEEL_DELTA);

			// Convert the screen coordinates to client coordinates.
			POINT clientToScreenPoint;
			clientToScreenPoint.x = x;
			clientToScreenPoint.y = y;
			ScreenToClient(hwnd, &clientToScreenPoint);

			MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, static_cast<int>(clientToScreenPoint.x), static_cast<int>(clientToScreenPoint.y), hwnd);
			engine->OnMouseWheel(mouseWheelEventArgs);
		}
		break;
		case WM_SIZE:
		{
			ResizeEventArgs resizeEventArgs(width, height, hwnd);
			engine->OnUpdateWindowSize(resizeEventArgs);
			if (wParam == SIZE_MINIMIZED)
			{
				Application->bIsAppPaused = true;
				Application->bIsAppMinimized = true;
				Application->bIsAppMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				Application->bIsAppPaused = false;
				Application->bIsAppMinimized = false;
				Application->bIsAppMaximized = true;
				engine->OnResizeRequest(hwnd);
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (Application->bIsAppMinimized)
				{
					Application->SetAppPaused(false);
					Application->bIsAppMinimized = false;
					engine->OnResizeRequest(hwnd);
				}

				// Restoring from maximized state?
				else if (Application->bIsAppMaximized)
				{
					Application->bIsAppPaused = false;
					Application->bIsAppMaximized = false;
					engine->OnResizeRequest(hwnd);
				}
				else if (Application->bIsResizing)
				{
					// If user is dragging the resize bars, we do not resize
					// the buffers here because as the user continuously
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is
					// done resizing the window and releases the resize bars, which
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					engine->OnResizeRequest(hwnd);
				}
			}
		}
		break;
		case WM_DESTROY:
		{
			// If a window is being destroyed, remove it from the
			// window maps.
			OEngine::RemoveWindow(hwnd);

			if (OEngine::WindowsMap.empty())
			{
				// If there are no more windows, quit the application.
				PostQuitMessage(0);
			}
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			app->bIsAppPaused = true;
			app->bIsResizing = true;
			app->Timer.Stop();
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			app->bIsAppPaused = false;
			app->bIsResizing = false;
			app->Timer.Start();
			engine->OnResizeRequest(hwnd);
			break;
		}
		case WM_MENUCHAR:
		{
			return MAKELRESULT(0, MNC_CLOSE);
		}
		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			break;
		}
		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
	}
	else
	{
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	return 0;
}

void OApplication::SetAppPaused(bool bPaused)
{
	bIsAppPaused = bPaused;
}

string OApplication::GetConfigPath(const string& Key) const
{
	return RootDirPath.GetPath() + ConfigReader->GetRoot<string>(Key);
}

wstring OApplication::GetResourcePath(const wstring& Resource) const
{
	return RootDirPath.GetWPath() + Resource;
}
const wstring& OApplication::GetShadersFolder() const
{
	static wstring ShadersFolder = L"";
	ShadersFolder = RootDirPath.GetWPath() + L"Shaders/";
	return ShadersFolder;
}