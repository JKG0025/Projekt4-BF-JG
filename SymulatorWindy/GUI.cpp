#include "GUI.h"
#pragma comment (lib,"Gdiplus.lib")

const wchar_t* GdiplusWindow::CLASS_NAME = L"GdiplusWindowClass";

void GdiplusWindow::InitializeGDIPlus() {
	Gdiplus::GdiplusStartupInput input;
	if (Gdiplus::GdiplusStartup(&gdiplusToken_, &input, nullptr) != Gdiplus::Ok)
		throw std::runtime_error("Failed to initialize GDI+.");
}

void GdiplusWindow::ShutdownGDIPlus() {
	if (gdiplusToken_)
		Gdiplus::GdiplusShutdown(gdiplusToken_);
}

void GdiplusWindow::RegisterWindowClass(HINSTANCE hInstance, const wchar_t* className) {
	static bool registered = false;
	if (registered) return;
	WNDCLASSW wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Optional: better painting behavior
	wc.lpfnWndProc = StaticWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	if (!RegisterClassW(&wc))
		throw std::runtime_error("Failed to register window class.");
	registered = true;
}

GdiplusWindow::GdiplusWindow(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height, const std::wstring& backgroundImagePath)
	: hInst_(hInstance) {
	InitializeGDIPlus();
	if (!backgroundImagePath.empty()) {
		background_ = std::make_unique<Gdiplus::Bitmap>(backgroundImagePath.c_str());
		if (background_->GetLastStatus() != Gdiplus::Ok)
			throw std::runtime_error("Failed to load background image.");
	}
	RegisterWindowClass(hInstance, CLASS_NAME);
	hWnd_ = CreateWindowExW(0, CLASS_NAME, windowTitle.c_str(),
		(WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, hInstance, this);
	if (!hWnd_)
		throw std::runtime_error("Failed to create window.");
}

GdiplusWindow::~GdiplusWindow() { ShutdownGDIPlus(); }

void GdiplusWindow::Show(int nCmdShow) {
	ShowWindow(hWnd_, nCmdShow);
	UpdateWindow(hWnd_);
}

int GdiplusWindow::RunMessageLoop() {
	MSG msg{};
	while (true) {
		if (MsgWaitForMultipleObjects(0, nullptr, FALSE, ANIMATION_TIMER_INTERVAL_MS, QS_ALLINPUT) == WAIT_FAILED)
			break;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) return (int)msg.wParam;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

GdiplusWindow::SpriteId GdiplusWindow::AddSprite(const std::wstring& imagePath, int x, int y) {
	auto img = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
	if (img->GetLastStatus() != Gdiplus::Ok) return (SpriteId)-1;
 
	Sprite s{
		nextSpriteId_++,
		std::move(img),
		Gdiplus::Point(x, y),
		Gdiplus::Size((INT)s.image->GetWidth(), (INT)s.image->GetHeight())
	};

	sprites_.push_back(std::move(s));
	InvalidateRect(hWnd_, nullptr, FALSE);
	return sprites_.back().id;
}

int GdiplusWindow::getSpriteX(SpriteId id) const
{
	for (const auto& s : sprites_) {
		if (s.id == id) return s.pos.X;
	}
	return -1; // Not found
}
int GdiplusWindow::getSpriteY(SpriteId id) const
{
	for (const auto& s : sprites_) {
		if (s.id == id) return s.pos.Y;
	}
	return -1; // Not found
}

void GdiplusWindow::RemoveSprite(SpriteId id) {
	auto it = std::remove_if(sprites_.begin(), sprites_.end(), [id](const Sprite& s) { return s.id == id; });
	if (it != sprites_.end()) {
		sprites_.erase(it, sprites_.end());
		InvalidateRect(hWnd_, nullptr, FALSE);
	}
}

void GdiplusWindow::MoveSprite(SpriteId id, int newX, int newY) {
	for (auto& s : sprites_) if (s.id == id) { s.pos = { newX, newY }; break; }
	InvalidateRect(hWnd_, nullptr, FALSE);
}

size_t GdiplusWindow::AddLine(int x1, int y1, int x2, int y2, Gdiplus::Color color, float thickness) {
	lines_.push_back({ {x1, y1}, {x2, y2}, color, thickness });
	InvalidateRect(hWnd_, nullptr, FALSE);
	return lines_.size() - 1;
}

void GdiplusWindow::RemoveLine(size_t lineIndex) {
	if (lineIndex < lines_.size()) {
		lines_.erase(lines_.begin() + lineIndex);
		InvalidateRect(hWnd_, nullptr, FALSE);
	}
}

void GdiplusWindow::AnimateLine(size_t lineIndex, int toX1, int toY1, int toX2, int toY2, int durationMs) {
	if (lineIndex >= lines_.size()) return;
	LineAnimation anim{ lines_[lineIndex], { {toX1, toY1}, {toX2, toY2}, lines_[lineIndex].color, lines_[lineIndex].thickness }, durationMs, std::chrono::steady_clock::now(), true };
	lineAnimations_[lineIndex] = anim;
	if (!animationTimerId_) StartAnimationTimer();
}

size_t GdiplusWindow::AddText(const std::wstring& text, int x, int y, const std::wstring& fontFamily, float fontSize, Gdiplus::Color color) {
	texts_.push_back({ text, {x, y}, fontFamily, fontSize, color });
	InvalidateRect(hWnd_, nullptr, FALSE);
	return texts_.size() - 1;
}

void GdiplusWindow::RemoveText(size_t textIndex) {
	if (textIndex < texts_.size()) {
		texts_.erase(texts_.begin() + textIndex);
		InvalidateRect(hWnd_, nullptr, FALSE);
	}
}

void GdiplusWindow::EditText(size_t textIndex, const std::wstring& newText, int newX, int newY, const std::wstring& newFontFamily, float newFontSize, Gdiplus::Color newColor) {
	if (textIndex < texts_.size()) {
		texts_[textIndex] = { newText, {newX, newY}, newFontFamily, newFontSize, newColor };
		InvalidateRect(hWnd_, nullptr, FALSE);
	}
}

HWND GdiplusWindow::AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb) {
	HWND btn = CreateWindowW(L"BUTTON", text.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, x, y, width, height, hWnd_, nullptr, hInst_, nullptr);
	if (btn) buttons_.push_back({ btn, std::move(cb) });
	return btn;
}

void GdiplusWindow::AnimateSprite(SpriteId id, int toX, int toY, float speedPxPerSec, bool deleteAfter) {
	auto it = std::find_if(sprites_.begin(), sprites_.end(), [id](const Sprite& s) { return s.id == id; });
	if (it == sprites_.end()) return;

	// Calculate distance
	int dx = toX - it->pos.X;
	int dy = toY - it->pos.Y;
	float distance = std::sqrt(static_cast<float>(dx * dx + dy * dy));

	// Calculate duration in ms based on speed (pixels per second)
	int durationMs = speedPxPerSec > 0.0f ? static_cast<int>((distance / speedPxPerSec) * 1000.0f) : 0;
	if (durationMs <= 0) {
		// Move instantly if speed is zero or negative
		it->pos = { toX, toY };
		InvalidateRect(hWnd_, nullptr, FALSE);
		return;
	}

	spriteAnimations_[id] = { it->pos, {toX, toY}, durationMs, std::chrono::steady_clock::now(), true };
	if (!animationTimerId_) StartAnimationTimer();
	if (deleteAfter) {
		std::thread([this, id]() {
			WaitForSpriteAnimation(id);
			RemoveSprite(id);
			}).detach();
	}
}


void GdiplusWindow::WaitForSpriteAnimation(SpriteId id)
{
	// Always wait until animation finishes, ignoring timeoutMs
	while (spriteAnimations_.find(id) != spriteAnimations_.end()) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid busy waiting
	}
}

void GdiplusWindow::WaitForDuration(int durationMs)
{
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(durationMs);
	while (std::chrono::steady_clock::now() < endTime) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid busy waiting
	}
}

void GdiplusWindow::StopSpriteAnimation(SpriteId id) {
	spriteAnimations_.erase(id);
	if (spriteAnimations_.empty()) StopAnimationTimer();
}

void GdiplusWindow::StopAllSpriteAnimations() {
	spriteAnimations_.clear();
	StopAnimationTimer();
}

LRESULT CALLBACK GdiplusWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	GdiplusWindow* pThis = nullptr;
	if (msg == WM_NCCREATE) {
		pThis = reinterpret_cast<GdiplusWindow*>(reinterpret_cast<CREATESTRUCT*>(lp)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
	}
	else {
		pThis = reinterpret_cast<GdiplusWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}
	return pThis ? pThis->WndProc(hwnd, msg, wp, lp) : DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT GdiplusWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		OnPaint(BeginPaint(hwnd, &ps));
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND: return 1;
	case WM_COMMAND: OnCommand(wp, lp); return 0;
	case WM_DESTROY: PostQuitMessage(0); return 0;
	case WM_TIMER:
		if (wp == animationTimerId_) { OnAnimationTimer(); return 0; }
		break;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

void GdiplusWindow::OnPaint(HDC hdc) {
	RECT rc; GetClientRect(hWnd_, &rc);
	int w = rc.right - rc.left, h = rc.bottom - rc.top;
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
	HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);
	HBRUSH back = CreateSolidBrush(RGB(255, 255, 255));
	FillRect(memDC, &rc, back);
	DeleteObject(back);
	Gdiplus::Graphics g(memDC);
	if (background_) g.DrawImage(background_.get(), 0, 0, w, h);
	for (const auto& s : sprites_) g.DrawImage(s.image.get(), s.pos.X, s.pos.Y, s.size.Width, s.size.Height);
	for (const auto& l : lines_) { Gdiplus::Pen pen(l.color, l.thickness); g.DrawLine(&pen, l.start, l.end); }
	for (const auto& t : texts_) {
		Gdiplus::FontFamily ff(t.fontFamily.c_str());
		Gdiplus::Font font(&ff, t.fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brush(t.color);
		g.DrawString(t.text.c_str(), -1, &font, Gdiplus::PointF((Gdiplus::REAL)t.pos.X, (Gdiplus::REAL)t.pos.Y), &brush);
	}
	BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
	SelectObject(memDC, oldBmp);
	DeleteObject(memBmp);
	DeleteDC(memDC);
}

void GdiplusWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(wParam) == BN_CLICKED) {
		HWND hBtn = (HWND)lParam;
		for (const auto& b : buttons_) if (b.hwnd == hBtn && b.cb) { b.cb(); break; }
	}
}

void GdiplusWindow::StartAnimationTimer() {
	animationTimerId_ = SetTimer(hWnd_, 1, ANIMATION_TIMER_INTERVAL_MS, nullptr);
	if (!animationTimerId_) throw std::runtime_error("Failed to create animation timer.");
}

void GdiplusWindow::StopAnimationTimer() {
	if (animationTimerId_) { KillTimer(hWnd_, animationTimerId_); animationTimerId_ = 0; }
}

void GdiplusWindow::OnAnimationTimer() {
	UpdateSpriteAnimations();
	InvalidateRect(hWnd_, nullptr, FALSE);
}

void GdiplusWindow::UpdateSpriteAnimations() {
	auto now = std::chrono::steady_clock::now();
	for (auto it = spriteAnimations_.begin(); it != spriteAnimations_.end();) {
		auto& anim = it->second;
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - anim.startTime).count();
		if (elapsed >= anim.durationMs) {
			for (auto& s : sprites_) if (s.id == it->first) { s.pos = anim.to; break; }
			it = spriteAnimations_.erase(it);
		}
		else {
			float p = (float)elapsed / anim.durationMs;
			Gdiplus::Point np(
				(INT)(anim.from.X + (anim.to.X - anim.from.X) * p),
				(INT)(anim.from.Y + (anim.to.Y - anim.from.Y) * p));
			for (auto& s : sprites_) if (s.id == it->first) { s.pos = np; break; }
			++it;
		}
	}
}
