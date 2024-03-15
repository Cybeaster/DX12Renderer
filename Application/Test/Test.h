#pragma once
#include "Engine/Engine.h"
#include "Events.h"

#include <Types.h>

class OEngine;

class OTest
{
public:
	virtual ~OTest() = default;

	OTest() = default;

	explicit OTest(OWindow* _Window)
	    : Engine(OEngine::Get())
	    , Window(_Window)
	{
	}

	virtual bool Initialize() { return true; }

	virtual void UnloadContent()
	{
	}

	virtual void Destroy();

	virtual void OnUpdate(const UpdateEventArgs& Args)
	{
	}

	virtual void OnRender(const UpdateEventArgs& Arg) {};

	virtual void OnWindowDestroyed();

	virtual void OnResize(const ResizeEventArgs& Args)
	{
	}

	virtual void OnKeyPressed(const KeyEventArgs& Args)
	{
	}

	virtual void OnMouseWheel(const MouseWheelEventArgs& Args)
	{
	}

	virtual void OnMouseMoved(const MouseMotionEventArgs& Args)
	{
	}

	virtual void OnMouseButtonPressed(const MouseButtonEventArgs& Args)
	{
	}

	virtual void OnMouseButtonReleased(const MouseButtonEventArgs& Args)
	{
	}

	auto GetWindow() const
	{
		return Window;
	}

protected:
	OEngine* GetEngine() const { return Engine; }

	OEngine* Engine;
	OWindow* Window;
};
