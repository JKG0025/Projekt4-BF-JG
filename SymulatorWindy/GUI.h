#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

// Strukturka reprezentuj¹ca "sprite" – obrazek z pozycj¹
struct Sprite {
    std::unique_ptr<Bitmap> image;
    POINT pos;
    SIZE size;
};

class GdiplusWindow {
public:
    using ButtonCallback = std::function<void()>;

    // Konstruktor: tworzy okno i ³aduje t³o
    GdiplusWindow(HINSTANCE hInstance,
        const std::wstring& windowTitle,
        int width,
        int height,
        const std::wstring& backgroundImagePath);

    // Destruktor: czyœci zasoby GDI+
    ~GdiplusWindow();

    // Pokazuje okno
    void Show(int nCmdShow = SW_SHOW);

    // Metody do zarz¹dzania sprite’ami
    size_t AddSprite(const std::wstring& imagePath, int x, int y);
    void MoveSprite(size_t spriteId, int newX, int newY);

    // Rysowanie linii i tekstu
    void DrawLine(int x1, int y1, int x2, int y2, int thickness = 1);
    void DrawText(const std::wstring& text, int x, int y, const std::wstring& fontFamily = L"Arial", int fontSize = 16);

    // Dodaje przycisk: zwraca jego HWND
    HWND AddButton(const std::wstring& text, int x, int y, int width, int height, ButtonCallback cb);

    // Pêtla komunikatów
    int RunMessageLoop();

private:
    // WindowProc statyczne i przekierowanie do metody instancji
    static LRESULT CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

    // Pomocnicze funkcje
    void OnPaint(HDC hdc);
    void OnCommand(WPARAM wParam);

    // Dane cz³onkowskie
    HINSTANCE            hInst_;
    HWND                 hWnd_;
    ULONG_PTR            gdiplusToken_;
    std::unique_ptr<Bitmap> background_;

    std::vector<Sprite>  sprites_;

    struct ButtonData { HWND hwnd; ButtonCallback cb; };
    std::vector<ButtonData> buttons_;
};
