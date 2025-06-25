#include "GUI.h"

// -----------------------------------------------------------------------------
// Rejestracja klasy okna wewn¹trz konstruktora
// -----------------------------------------------------------------------------
GdiplusWindow::GdiplusWindow(HINSTANCE hInstance,
	const std::wstring& windowTitle,
	int width,
	int height,
	const std::wstring& backgroundImagePath)
	: hInst_(hInstance)
{
	// 1) Start GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, nullptr);

	// 2) Zarejestruj klasê okna
	WNDCLASS wc = {};
	wc.lpfnWndProc = StaticWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("GdiplusWindowClass");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;  // t³o malujemy sami
	RegisterClass(&wc);

	// 3) Stwórz okno
	hWnd_ = CreateWindowEx(
		0,
		wc.lpszClassName,
		(LPCSTR)windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX,  // niezmienna wielkoœæ
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		nullptr, nullptr, hInstance, this
	);

	// 4) Za³aduj t³o
	background_ = std::make_unique<Bitmap>(backgroundImagePath.c_str());
}

// -----------------------------------------------------------------------------
GdiplusWindow::~GdiplusWindow() {
	GdiplusShutdown(gdiplusToken_);
}

// -----------------------------------------------------------------------------
void GdiplusWindow::Show(int nCmdShow) {
	ShowWindow(hWnd_, nCmdShow);
	UpdateWindow(hWnd_);
}

// -----------------------------------------------------------------------------
int GdiplusWindow::RunMessageLoop() {
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}

// -----------------------------------------------------------------------------
size_t GdiplusWindow::AddSprite(const std::wstring& imagePath, int x, int y) {
	Sprite s;
	s.image = std::make_unique<Bitmap>(imagePath.c_str());
	s.pos = { x, y };
	s.size = { (long)s.image->GetWidth(), (long)s.image->GetHeight() };
	sprites_.push_back(std::move(s));
	InvalidateRect(hWnd_, nullptr, FALSE);
	return sprites_.size() - 1;
}

// -----------------------------------------------------------------------------
void GdiplusWindow::MoveSprite(size_t id, int newX, int newY) {
	if (id < sprites_.size()) {
		sprites_[id].pos = { newX, newY };
		InvalidateRect(hWnd_, nullptr, FALSE);
	}
}

// -----------------------------------------------------------------------------
void GdiplusWindow::DrawLine(int x1, int y1, int x2, int y2, int thickness) {
	// zapisujemy, a rysujemy przy najbli¿szym WM_PAINT
	HPEN pen = CreatePen(PS_SOLID, thickness, RGB(255, 0, 0));
	SelectObject(GetDC(hWnd_), pen);
	MoveToEx(GetDC(hWnd_), x1, y1, nullptr);
	LineTo(GetDC(hWnd_), x2, y2);
	DeleteObject(pen);
}

// -----------------------------------------------------------------------------
void GdiplusWindow::DrawText(const std::wstring& text, int x, int y, const std::wstring& fontFamily, int fontSize) {
	HDC hdc = GetDC(hWnd_);
	::TextOutW(hdc, x, y, text.c_str(), (int)text.size());
	ReleaseDC(hWnd_, hdc);
}

// -----------------------------------------------------------------------------
HWND GdiplusWindow::AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb) {
	HWND btn = CreateWindowEx(
		0, TEXT("BUTTON"), (LPCSTR)text.c_str(),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, width, height,
		hWnd_, nullptr, hInst_, nullptr
	);
	buttons_.push_back({ btn, cb });
	return btn;
}

// -----------------------------------------------------------------------------
LRESULT CALLBACK GdiplusWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (msg == WM_NCCREATE) {
		// Przy pierwszym CREATE wska¿ instancjê klasy jako user data
		CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lp);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}
	auto* that = reinterpret_cast<GdiplusWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (that)
		return that->WndProc(hwnd, msg, wp, lp);
	else
		return DefWindowProc(hwnd, msg, wp, lp);
}

// -----------------------------------------------------------------------------
LRESULT GdiplusWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		OnPaint(hdc);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_COMMAND:
		OnCommand(wp);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

// -----------------------------------------------------------------------------
void GdiplusWindow::OnPaint(HDC hdc) {
	Graphics g(hdc);
	// 1) t³o
	g.DrawImage(background_.get(), 0, 0,
		background_->GetWidth(), background_->GetHeight());
	// 2) wszystkie sprite’y
	for (auto& s : sprites_) {
		g.DrawImage(s.image.get(), s.pos.x, s.pos.y,
			s.size.cx, s.size.cy);
	}
	// (Mo¿esz tu te¿ wypisaæ pamiêtne rysowania linii/tekstu
	//  trzymane w wektorach, jeœli chcesz bardziej trwa³ego zapisu)
}

// -----------------------------------------------------------------------------
void GdiplusWindow::OnCommand(WPARAM wParam) {
	HWND src = HWND(LOWORD(wParam));
	for (auto& b : buttons_) {
		if (b.hwnd == src && b.cb) {
			b.cb();
			break;
		}
	}
}
