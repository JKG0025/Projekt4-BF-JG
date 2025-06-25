#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <unordered_map>

class GdiplusWindow {
public:
	using ButtonCallback = std::function<void()>;
	using SpriteId = size_t;

	GdiplusWindow(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height, const std::wstring& backgroundImagePath = L"");
	~GdiplusWindow();

	GdiplusWindow(const GdiplusWindow&) = delete;
	GdiplusWindow& operator=(const GdiplusWindow&) = delete;

	void Show(int nCmdShow = SW_SHOW);
	int RunMessageLoop();

	SpriteId AddSprite(const std::wstring& imagePath, int x, int y);
	void MoveSprite(SpriteId id, int newX, int newY);
	void AnimateSprite(SpriteId id, int toX, int toY, int durationMs);
	void StopSpriteAnimation(SpriteId id);
	void StopAllSpriteAnimations();

	void AddLine(int x1, int y1, int x2, int y2, Gdiplus::Color color, float thickness = 1.0f);
	void AddText(const std::wstring& text, int x, int y, const std::wstring& fontFamily, float fontSize, Gdiplus::Color color);

	HWND AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb);
	HWND GetWindowHandle() const { return hWnd_; }

private:
	struct Sprite 
	{
		SpriteId id;
		std::unique_ptr<Gdiplus::Bitmap> image;
		Gdiplus::Point pos;
		Gdiplus::Size size;
	};

	struct ButtonInfo 
	{
		HWND hwnd;
		ButtonCallback cb;
	};

	struct Line 
	{
		Gdiplus::Point start;
		Gdiplus::Point end;
		Gdiplus::Color color;
		float thickness;
	};

	struct Text 
	{
		std::wstring text;
		Gdiplus::Point pos;
		std::wstring fontFamily;
		float fontSize;
		Gdiplus::Color color;
	};

	struct SpriteAnimation 
	{
		Gdiplus::Point from;
		Gdiplus::Point to;
		int durationMs;
		std::chrono::steady_clock::time_point startTime;
		bool active = false;
	};

	// Window procedure and message handlers
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	void OnPaint(HDC hdc);
	void OnCommand(WPARAM wParam, LPARAM lParam);

	// Animation timer
	void StartAnimationTimer();
	void StopAnimationTimer();
	void OnAnimationTimer();
	void UpdateSpriteAnimations();

	// Helpers
	static void RegisterWindowClass(HINSTANCE hInstance, const wchar_t* className);
	void InitializeGDIPlus();
	void ShutdownGDIPlus();

	// Member variables
	HWND hWnd_ = nullptr;
	HINSTANCE hInst_ = nullptr;
	ULONG_PTR gdiplusToken_ = 0;

	std::unique_ptr<Gdiplus::Bitmap> background_;
	std::vector<Sprite> sprites_;
	std::vector<ButtonInfo> buttons_;
	std::vector<Line> lines_;
	std::vector<Text> texts_;

	SpriteId nextSpriteId_ = 0;
	std::unordered_map<SpriteId, SpriteAnimation> spriteAnimations_;
	UINT_PTR animationTimerId_ = 0;

	static constexpr UINT ANIMATION_TIMER_INTERVAL_MS = 16;
	static const wchar_t* CLASS_NAME;
};