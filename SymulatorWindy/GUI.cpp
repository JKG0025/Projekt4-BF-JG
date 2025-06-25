#include "GUI.h"
#include <stdexcept>
#include <objidl.h> // For Gdiplus
#pragma comment (lib,"Gdiplus.lib")

const wchar_t* GdiplusWindow::CLASS_NAME = L"GdiplusWindowClass";

void GdiplusWindow::InitializeGDIPlus()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if (Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, nullptr) != Gdiplus::Ok) {
		throw std::runtime_error("Failed to initialize GDI+.");
	}
}

void GdiplusWindow::ShutdownGDIPlus() {
	if (gdiplusToken_ != 0) {
		Gdiplus::GdiplusShutdown(gdiplusToken_);
	}
}

void GdiplusWindow::RegisterWindowClass(HINSTANCE hInstance, const wchar_t* className) {
	static bool isRegistered = false;
	if (isRegistered) return;

	WNDCLASSW wc = {};
	wc.lpfnWndProc = StaticWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	if (!RegisterClassW(&wc)) {
		throw std::runtime_error("Failed to register window class.");
	}
	isRegistered = true;
}

GdiplusWindow::GdiplusWindow(HINSTANCE hInstance,
	const std::wstring& windowTitle,
	int width,
	int height,
	const std::wstring& backgroundImagePath)
	: hInst_(hInstance)
{
	InitializeGDIPlus();

	if (!backgroundImagePath.empty()) {
		background_ = std::make_unique<Gdiplus::Bitmap>(backgroundImagePath.c_str());
		if (background_->GetLastStatus() != Gdiplus::Ok) {
			background_.reset();
			throw std::runtime_error("Failed to load background image.");
		}
	}

	RegisterWindowClass(hInstance, CLASS_NAME);

	hWnd_ = CreateWindowExW(
		0,
		CLASS_NAME,
		windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		nullptr,
		nullptr,
		hInstance,
		this
	);

	if (!hWnd_) {
		ShutdownGDIPlus();
		throw std::runtime_error("Failed to create window.");
	}
}

GdiplusWindow::~GdiplusWindow() {
	ShutdownGDIPlus();
}

void GdiplusWindow::Show(int nCmdShow) {
	ShowWindow(hWnd_, nCmdShow);
	UpdateWindow(hWnd_);
}

int GdiplusWindow::RunMessageLoop() {
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}

GdiplusWindow::SpriteId GdiplusWindow::AddSprite(const std::wstring& imagePath, int x, int y) {
	auto image = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
	if (image->GetLastStatus() != Gdiplus::Ok) {
		return -1;
	}

	Sprite s;
	s.id = nextSpriteId_++;
	s.pos = { x, y };
	s.size = { static_cast<INT>(image->GetWidth()), static_cast<INT>(image->GetHeight()) };
	s.image = std::move(image);

	sprites_.push_back(std::move(s));

	InvalidateRect(hWnd_, nullptr, FALSE);
	return s.id;
}

void GdiplusWindow::MoveSprite(SpriteId id, int newX, int newY) {
	for (auto& sprite : sprites_) {
		if (sprite.id == id) {
			sprite.pos = { newX, newY };
			InvalidateRect(hWnd_, nullptr, FALSE);
			return;
		}
	}
}

void GdiplusWindow::AddLine(int x1, int y1, int x2, int y2, Gdiplus::Color color, float thickness) {
	lines_.push_back({ {x1, y1}, {x2, y2}, color, thickness });
	InvalidateRect(hWnd_, nullptr, FALSE);
}

void GdiplusWindow::AddText(const std::wstring& text, int x, int y, const std::wstring& fontFamily, float fontSize, Gdiplus::Color color) {
	texts_.push_back({ text, {x, y}, fontFamily, fontSize, color });
	InvalidateRect(hWnd_, nullptr, FALSE);
}

HWND GdiplusWindow::AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb) {
	HWND btnHwnd = CreateWindowW(
		L"BUTTON",
		text.c_str(),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, width, height,
		hWnd_,
		nullptr,
		hInst_,
		nullptr);

	if (btnHwnd) {
		buttons_.push_back({ btnHwnd, std::move(cb) });
	}
	return btnHwnd;
}

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

void GdiplusWindow::OnPaint(HDC hdc) {
	Gdiplus::Graphics screenGraphics(hdc);

	RECT rc;
	GetClientRect(hWnd_, &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	Gdiplus::Bitmap backBuffer(width, height, &screenGraphics);
	Gdiplus::Graphics bufferGraphics(&backBuffer);

	bufferGraphics.Clear(Gdiplus::Color::White);

	if (background_) {
		bufferGraphics.DrawImage(background_.get(), 0, 0, width, height);
	}

	for (const auto& s : sprites_) {
		bufferGraphics.DrawImage(s.image.get(), s.pos.X, s.pos.Y, s.size.Width, s.size.Height);
	}

	for (const auto& line : lines_) {
		Gdiplus::Pen pen(line.color, line.thickness);
		bufferGraphics.DrawLine(&pen, line.start, line.end);
	}

	for (const auto& text : texts_) {
		Gdiplus::FontFamily fontFamily(text.fontFamily.c_str());
		Gdiplus::Font font(&fontFamily, text.fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brush(text.color);
		bufferGraphics.DrawString(text.text.c_str(), -1, &font, Gdiplus::PointF((Gdiplus::REAL)text.pos.X, (Gdiplus::REAL)text.pos.Y), &brush);
	}

	screenGraphics.DrawImage(&backBuffer, 0, 0);
}

void GdiplusWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(wParam) == BN_CLICKED) {
		HWND hButton = (HWND)lParam;
		for (const auto& b : buttons_) {
			if (b.hwnd == hButton && b.cb) {
				b.cb();
				break;
			}
		}
	}
}
