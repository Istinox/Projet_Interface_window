#include "ImageManager.h"
#include <fstream>
#include <vector>
#include <cstring>

HBITMAP ImageManager::LoadFromFile(const std::wstring& path, std::wstring& err) {
    HBITMAP hbm = (HBITMAP)LoadImageW(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (!hbm) {
        err = L"Échec du chargement de l'image.";
    }
    return hbm;
}

bool ImageManager::SaveToFile(HBITMAP hBitmap, const std::wstring& path, std::wstring& err) {
    if (!hBitmap) { err = L"Aucun bitmap à sauvegarder"; return false; }

    BITMAP bmp = {};
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) { err = L"GetObject échoué"; return false; }

    // Préparer header pour écrire en 24bpp
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // enregistrer en 24bpp
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;

    int rowSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4;
    int imageSize = rowSize * bmp.bmHeight;
    std::vector<BYTE> pixels(imageSize);

    BITMAPINFO bmi = {};
    bmi.bmiHeader = bi;

    HDC hdc = GetDC(NULL);
    if (GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc);
        err = L"GetDIBits échoué";
        return false;
    }
    ReleaseDC(NULL, hdc);

    // Préparer BITMAPFILEHEADER
    BITMAPFILEHEADER bfh = {};
    bfh.bfType = 0x4D42; // 'BM'
    // bfOffBits doit être aligné : sizeof(BITMAPFILEHEADER) + size of BITMAPINFOHEADER
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = (DWORD)(bfh.bfOffBits + imageSize);

    // écrire fichier
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) { err = L"Impossible d'ouvrir le fichier en écriture"; return false; }
    ofs.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
    ofs.write(reinterpret_cast<const char*>(&bi), sizeof(bi));
    ofs.write(reinterpret_cast<const char*>(pixels.data()), imageSize);
    ofs.close();
    return true;
}

void ImageManager::AddTitle(HDC hdcTitle, HWND hwnd, PAINTSTRUCT& psTitle, const char* title, SIZE& sizeTitle, int xTitle, int yTitle) {
    HFONT hfontTitle = CreateFontA(
        40, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial"
    );

    hdcTitle = BeginPaint(hwnd, &psTitle);
    GetTextExtentPoint32A(hdcTitle, title, (int)strlen(title), &sizeTitle);
    SelectObject(hdcTitle, hfontTitle);
    TextOutA(hdcTitle, xTitle, yTitle, title, (int)strlen(title));
    EndPaint(hwnd, &psTitle);

    DeleteObject(hfontTitle);
}

void ImageManager::DrawBMPFile(HWND hwnd, HDC hdc, HBITMAP hBitmap) {
    if (hBitmap) {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

        BITMAP bmp;
        GetObject(hBitmap, sizeof(bmp), &bmp);

        // On dessine à la position (250,50) comme dans ton code initial
        BitBlt(hdc, 250, 50, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hOld);
        DeleteDC(hdcMem);
    }
}
