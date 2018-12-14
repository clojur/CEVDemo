#pragma once

struct IViewport
{
	virtual int			GetWidth() { return 0; }

	virtual int			GetHeight() { return 0; }

	virtual void*		GetSafeHwnd() = 0;

	virtual	void		OnSize(int width, int height) = 0;

	virtual	void		OnMove(int x, int y) {};

	virtual bool		InitializeWindow() { return false; };

	virtual void		Update() {};

	virtual void		DestoryWindow() {};
};