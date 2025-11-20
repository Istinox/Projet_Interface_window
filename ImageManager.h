//#pragma once
//#include <windows.h>
//
//class ImageManager
//{
//public:
//	void AddTitle(HDC hdcTitle, HWND hwnd, PAINTSTRUCT psTitle, const char* title, SIZE sizeTitle, int xTitle, int yTitle) {
//        HFONT hfontTitle = CreateFontA(
//            40,                        // taille du texte (en pixels)
//            0,                         // largeur (0 = automatique)
//            0, 0,                      // angle d'orientation (x, y)
//            FW_NORMAL,                 // texte en gras
//            FALSE,                     // italique
//            FALSE,                     // souligné
//            FALSE,                     // barré
//            DEFAULT_CHARSET,           // charset
//            OUT_CHARACTER_PRECIS,      // précision de sortie
//            CLIP_CHARACTER_PRECIS,     // précision du clipping du texte
//            CLEARTYPE_QUALITY,         // qualité du rendu du texte
//            DEFAULT_PITCH | FF_DONTCARE, // ???
//            "Arial"                    // nom de la police
//        );
//
//        hdcTitle = BeginPaint(hwnd, &psTitle);
//        GetTextExtentPoint32A(hdcTitle, title, (int)strlen(title), &sizeTitle);
//
//        (HFONT)SelectObject(hdcTitle, hfontTitle);
//        TextOutA(hdcTitle, xTitle, yTitle, title, (int)strlen(title));
//        EndPaint(hwnd, &psTitle);
//	}
//
//    void DrawBMPFile(HWND hwnd, HDC hdc, HBITMAP hBitmap) {
//
//        if (hBitmap) {
//            HDC hdcMem = CreateCompatibleDC(hdc);
//            HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);
//
//            BITMAP bmp;
//            GetObject(hBitmap, sizeof(bmp), &bmp);
//
//            BitBlt(hdc, 250, 50, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);
//
//            SelectObject(hdcMem, hOld);
//            DeleteDC(hdcMem);
//        }
//    }
//
//    void SaveBMPFile() {
//    }
//};





#pragma once
#include <windows.h>
#include <string>

class ImageManager {
public:
    // retourne HBITMAP (caller doit garder et libérer via DeleteObject)
    HBITMAP LoadFromFile(const std::wstring& path, std::wstring& err);
    bool SaveToFile(HBITMAP hBitmap, const std::wstring& path, std::wstring& err);
    // Méthodes UI / dessin
    void AddTitle(HDC hdcTitle, HWND hwnd, PAINTSTRUCT& psTitle, const char* title, SIZE& sizeTitle, int xTitle, int yTitle);
    void DrawBMPFile(HWND hwnd, HDC hdc, HBITMAP hBitmap);
};


