#include "GUI.h"
#include <stdexcept>
#include <objidl.h> // For Gdiplus
#pragma comment (lib,"Gdiplus.lib")

// Define the static class name
const wchar_t* GdiplusWindow::CLASS_NAME = L"GdiplusWindowClass";

// -----------------------------------------------------------------------------
// GDI+ Initialization (called from constructor)
// -----------------------------------------------------------------------------
void GdiplusWindow::InitializeGDIPlus() {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if (Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, nullptr) != Gdiplus::Ok) {
		throw std::runtime_error("Failed to initialize GDI+.");
	}
}

// -----------------------------------------------------------------------------
// GDI+ Shutdown (called from destructor)
// -----------------------------------------------------------------------------
void GdiplusWindow::ShutdownGDIPlus() {
	if (gdiplusToken_ != 0) {
		Gdiplus::GdiplusShutdown(gdiplusToken_);
	}
}

// -----------------------------------------------------------------------------
// Registers the window class. Done only once.
// -----------------------------------------------------------------------------
void GdiplusWindow::RegisterWindowClass(HINSTANCE hInstance, const wchar_t* className) {
	// static ensures this runs only once per application lifetime.
	static bool isRegistered = false;
	if (isRegistered) return;

	WNDCLASSW wc = {};
	wc.lpfnWndProc = StaticWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default background

	if (!RegisterClassW(&wc)) {
		throw std::runtime_error("Failed to register window class.");
	}
	isRegistered = true;
}

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
GdiplusWindow::GdiplusWindow(HINSTANCE hInstance,
	const std::wstring& windowTitle,
	int width,
	int height,
	const std::wstring& backgroundImagePath)
	: hInst_(hInstance)
{
	InitializeGDIPlus();

	// Load background image if a path is provided
	if (!backgroundImagePath.empty()) {
		background_ = std::make_unique<Gdiplus::Bitmap>(backgroundImagePath.c_str());
		if (background_->GetLastStatus() != Gdiplus::Ok) {
			// Handle error: e.g., throw exception or load a default
			background_.reset(); // or throw std::runtime_error("Failed to load background image.");
		}
	}

	RegisterWindowClass(hInstance, CLASS_NAME);

	// Create the window
	hWnd_ = CreateWindowExW(
		0,                                  // Optional window styles.
		CLASS_NAME,                         // Window class
		windowTitle.c_str(),                // Window text
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX,  // Window style (non-resizable)
		CW_USEDEFAULT, CW_USEDEFAULT,       // Position
		width, height,                      // Size
		nullptr,                            // Parent window
		nullptr,                            // Menu
		hInstance,                          // Instance handle
		this                                // Additional application data
	);

	if (!hWnd_) {
		ShutdownGDIPlus(); // Clean up GDI+ before throwing
		throw std::runtime_error("Failed to create window.");
	}
}

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
GdiplusWindow::~GdiplusWindow() {
	ShutdownGDIPlus();
	// Note: The window is destroyed automatically by the OS. 
	// DestroyWindow(hWnd_) could be called here if explicit destruction is needed.
}

// -----------------------------------------------------------------------------
void GdiplusWindow::Show(int nCmdShow) {
	ShowWindow(hWnd_, nCmdShow);
	UpdateWindow(hWnd_);
}

// -----------------------------------------------------------------------------
int GdiplusWindow::RunMessageLoop() {
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}

// -----------------------------------------------------------------------------
GdiplusWindow::SpriteId GdiplusWindow::AddSprite(const std::wstring& imagePath, int x, int y) {
	auto image = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
	if (image->GetLastStatus() != Gdiplus::Ok) {
		// Return an invalid ID or throw an exception on failure
		return -1;
	}

	Sprite s;
	s.id = nextSpriteId_++;
	s.pos = { x, y };
	s.size = { static_cast<INT>(image->GetWidth()), static_cast<INT>(image->GetHeight()) };
	s.image = std::move(image);

	sprites_.push_back(std::move(s));

	InvalidateRect(hWnd_, nullptr, FALSE); // Request a redraw
	return s.id;
}

// -----------------------------------------------------------------------------
void GdiplusWindow::MoveSprite(SpriteId id, int newX, int newY) {
	for (auto& sprite : sprites_) {
		if (sprite.id == id) {
			sprite.pos = { newX, newY };
			InvalidateRect(hWnd_, nullptr, FALSE); // Request a redraw
			return;
		}
	}
}

// -----------------------------------------------------------------------------
// FIXED: Stores a line to be drawn during the next WM_PAINT event.
// -----------------------------------------------------------------------------
void GdiplusWindow::AddLine(int x1, int y1, int x2, int y2, Gdiplus::Color color, float thickness) {
	lines_.push_back({ {x1, y1}, {x2, y2}, color, thickness });
	InvalidateRect(hWnd_, nullptr, FALSE); // Request a redraw
}

// -----------------------------------------------------------------------------
// FIXED: Stores text to be drawn during the next WM_PAINT event.
// -----------------------------------------------------------------------------
void GdiplusWindow::AddText(const std::wstring& text, int x, int y, const std::wstring& fontFamily, float fontSize, Gdiplus::Color color) {
	texts_.push_back({ text, {x, y}, fontFamily, fontSize, color });
	InvalidateRect(hWnd_, nullptr, FALSE); // Request a redraw
}

// -----------------------------------------------------------------------------
// FIXED: Correctly handles wide strings for the button text.
// -----------------------------------------------------------------------------
HWND GdiplusWindow::AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb) {
	HWND btnHwnd = CreateWindowW(
		L"BUTTON",                          // Predefined class; Unicode
		text.c_str(),                       // Button text
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, width, height,
		hWnd_,                              // Parent window
		nullptr,                            // No menu.
		hInst_,
		nullptr);

	if (btnHwnd) {
		buttons_.push_back({ btnHwnd, std::move(cb) });
	}
	return btnHwnd;
}

// -----------------------------------------------------------------------------
// Static window procedure to forward messages to the correct class instance.
// -----------------------------------------------------------------------------
LRESULT CALLBACK GdiplusWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	GdiplusWindow* pThis = nullptr;

	if (msg == WM_NCCREATE) {
		CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lp);
		pThis = reinterpret_cast<GdiplusWindow*>(pCS->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
	}
	else {
		pThis = reinterpret_cast<GdiplusWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (pThis) {
		return pThis->WndProc(hwnd, msg, wp, lp);
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

// -----------------------------------------------------------------------------
// Member window procedure to handle messages for this window instance.
// -----------------------------------------------------------------------------
LRESULT GdiplusWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		OnPaint(hdc);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_COMMAND:
		OnCommand(wp, lp);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

// -----------------------------------------------------------------------------
// FIXED: Implements double-buffering for flicker-free rendering.
// All drawing now happens here.
// -----------------------------------------------------------------------------
void GdiplusWindow::OnPaint(HDC hdc) {
	Gdiplus::Graphics screenGraphics(hdc);

	// Get client area dimensions
	RECT rc;
	GetClientRect(hWnd_, &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	// Create a back buffer for double-buffering
	Gdiplus::Bitmap backBuffer(width, height, &screenGraphics);
	Gdiplus::Graphics bufferGraphics(&backBuffer);

	// ---- Start Drawing to the Back Buffer ----
	// Clear the buffer with a default color (or leave it transparent)
	bufferGraphics.Clear(Gdiplus::Color::White);

	// 1) Draw the background image
	if (background_) {
		bufferGraphics.DrawImage(background_.get(), 0, 0, width, height);
	}

	// 2) Draw all sprites
	for (const auto& s : sprites_) {
		bufferGraphics.DrawImage(s.image.get(), s.pos.X, s.pos.Y, s.size.Width, s.size.Height);
	}

	// 3) Draw all persistent lines
	for (const auto& line : lines_) {
		Gdiplus::Pen pen(line.color, line.thickness);
		bufferGraphics.DrawLine(&pen, line.start, line.end);
	}

	// 4) Draw all persistent text
	for (const auto& text : texts_) {
		Gdiplus::FontFamily fontFamily(text.fontFamily.c_str());
		Gdiplus::Font font(&fontFamily, text.fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brush(text.color);
		bufferGraphics.DrawString(text.text.c_str(), -1, &font, Gdiplus::PointF((Gdiplus::REAL)text.pos.X, (Gdiplus::REAL)text.pos.Y), &brush);
	}

	// ---- End Drawing to the Back Buffer ----

	// Copy the back buffer to the screen in one operation
	screenGraphics.DrawImage(&backBuffer, 0, 0);
}

// -----------------------------------------------------------------------------
// FIXED: More robustly handles button click notifications.
// -----------------------------------------------------------------------------
void GdiplusWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	// We only care about button clicks
	if (HIWORD(wParam) == BN_CLICKED) {
		HWND hButton = (HWND)lParam; // Handle of the button is in lParam
		for (const auto& b : buttons_) {
			if (b.hwnd == hButton && b.cb) {
				b.cb(); // Execute the callback
				break;
			}
		}
	}
}