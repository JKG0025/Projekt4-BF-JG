#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Forward declaration to avoid including gdiplus.h in user code if not needed
namespace Gdiplus
{
	class Bitmap;
	class Graphics;
}

class GdiplusWindow {
public:
	// Type alias for a button click callback function
	using ButtonCallback = std::function<void()>;
	// A simple, unique ID for sprites to avoid fragile indices
	using SpriteId = size_t;

	// Constructor & Destructor
	GdiplusWindow(HINSTANCE hInstance,
		const std::wstring& windowTitle,
		int width,
		int height,
		const std::wstring& backgroundImagePath = L"");
	~GdiplusWindow();

	// Prevent copying and assignment
	GdiplusWindow(const GdiplusWindow&) = delete;
	GdiplusWindow& operator=(const GdiplusWindow&) = delete;

	// Public interface
	void Show(int nCmdShow = SW_SHOW);
	int RunMessageLoop();

	SpriteId AddSprite(const std::wstring& imagePath, int x, int y);
	void MoveSprite(SpriteId id, int newX, int newY);

	// These functions now store shapes to be drawn during OnPaint
	void AddLine(int x1, int y1, int x2, int y2, Gdiplus::Color color, float thickness = 1.0f);
	void AddText(const std::wstring& text, int x, int y, const std::wstring& fontFamily, float fontSize, Gdiplus::Color color);

	HWND AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb);

	HWND GetWindowHandle() const { return hWnd_; }

private:
	// --- Internal Structures ---
	struct Sprite {
		SpriteId id;
		std::unique_ptr<Gdiplus::Bitmap> image;
		Gdiplus::Point pos;
		Gdiplus::Size size;
	};

	struct ButtonInfo {
		HWND hwnd;
		ButtonCallback cb;
	};

	struct Line {
		Gdiplus::Point start;
		Gdiplus::Point end;
		Gdiplus::Color color;
		float thickness;
	};

	struct Text {
		std::wstring text;
		Gdiplus::Point pos;
		std::wstring fontFamily;
		float fontSize;
		Gdiplus::Color color;
	};

	// --- Window Procedure and Message Handlers ---
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	void OnPaint(HDC hdc);
	void OnCommand(WPARAM wParam, LPARAM lParam);

	// --- Private Helper Methods ---
	static void RegisterWindowClass(HINSTANCE hInstance, const wchar_t* className);
	void InitializeGDIPlus();
	void ShutdownGDIPlus();

	// --- Member Variables ---
	HWND hWnd_ = nullptr;
	HINSTANCE hInst_ = nullptr;
	ULONG_PTR gdiplusToken_ = 0;

	std::unique_ptr<Gdiplus::Bitmap> background_;
	std::vector<Sprite> sprites_;
	std::vector<ButtonInfo> buttons_;
	std::vector<Line> lines_;
	std::vector<Text> texts_;

	SpriteId nextSpriteId_ = 0;

	static const wchar_t* CLASS_NAME;
};