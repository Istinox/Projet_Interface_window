#pragma once
#include <windows.h>
#include <string>

class ImageManager {
public:
    // retourne HBITMAP (caller doit garder et libérer via DeleteObject)
    HBITMAP LoadFromFile(const std::wstring& path, std::wstring& err);
    bool SaveToFile(HBITMAP hBitmap, const std::wstring& path, std::wstring& err);
    // Méthodes UI / dessin
    void AddTitle(HDC hdcTitle, HWND hwnd, PAINTSTRUCT psTitle, const char* title, SIZE sizeTitle, int xTitle, int yTitle);
    void DrawBMPFile(HWND hwnd, HDC hdc, HBITMAP hBitmap);
};


