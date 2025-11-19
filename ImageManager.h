#pragma once
#include <windows.h>

class ImageManager
{
public:
	void AddTitle(HDC hdcTitle, HWND hwnd, PAINTSTRUCT psTitle, const char* title, SIZE sizeTitle, int xTitle, int yTitle) {
        HFONT hfontTitle = CreateFontA(
            40,                        // taille du texte (en pixels)
            0,                         // largeur (0 = automatique)
            0, 0,                      // angle d'orientation (x, y)
            FW_NORMAL,                 // texte en gras
            FALSE,                     // italique
            FALSE,                     // souligné
            FALSE,                     // barré
            DEFAULT_CHARSET,           // charset
            OUT_CHARACTER_PRECIS,      // précision de sortie
            CLIP_CHARACTER_PRECIS,     // précision du clipping du texte
            CLEARTYPE_QUALITY,         // qualité du rendu du texte
            DEFAULT_PITCH | FF_DONTCARE, // ???
            "Arial"                    // nom de la police
        );

        hdcTitle = BeginPaint(hwnd, &psTitle);
        GetTextExtentPoint32A(hdcTitle, title, (int)strlen(title), &sizeTitle);

        (HFONT)SelectObject(hdcTitle, hfontTitle);
        TextOutA(hdcTitle, xTitle, yTitle, title, (int)strlen(title));
        EndPaint(hwnd, &psTitle);
	}

    void DrawBMPFile(HWND hwnd, HDC hdc, HBITMAP hBitmap) {

        if (hBitmap) {
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

            BITMAP bmp;
            GetObject(hBitmap, sizeof(bmp), &bmp);

            BitBlt(hdc, 250, 50, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOld);
            DeleteDC(hdcMem);
        }
    }

    void SaveBMPFile(HBITMAP hBitmap, HDC hdc, const char* filename) {
        BITMAP bmp;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        BITMAPFILEHEADER bmfHeader;
        BITMAPINFOHEADER biHeader;

        biHeader.biSize = sizeof(BITMAPINFOHEADER);
        biHeader.biWidth = bmp.bmWidth;
        biHeader.biHeight = bmp.bmHeight;
        biHeader.biPlanes = 1;
        biHeader.biBitCount = 24; // le nombre de bits pour le RGB (8 bits par couleur)
        biHeader.biCompression = BI_RGB; // (pas de compression)
        biHeader.biSizeImage = ((bmp.bmWidth * biHeader.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

        DWORD dwBmpSize = biHeader.biSizeImage;

        // Allocation mémoire pour les pixels
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);

        // Récupération des pixels
        GetDIBits(hdc, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&biHeader, DIB_RGB_COLORS);

        // Préparation de l’en-tête du fichier
        HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        DWORD dwSizeofFileHeader = sizeof(BITMAPFILEHEADER);
        DWORD dwSizeofInfoHeader = sizeof(BITMAPINFOHEADER);

        bmfHeader.bfOffBits = dwSizeofFileHeader + dwSizeofInfoHeader;
        bmfHeader.bfSize = dwSizeofFileHeader + dwSizeofInfoHeader + dwBmpSize;
        bmfHeader.bfType = 0x4D42; // "BM"

        DWORD dwWritten;
        WriteFile(hFile, (LPSTR)&bmfHeader, dwSizeofFileHeader, &dwWritten, NULL);
        WriteFile(hFile, (LPSTR)&biHeader, dwSizeofInfoHeader, &dwWritten, NULL);
        WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwWritten, NULL);
    }
};

