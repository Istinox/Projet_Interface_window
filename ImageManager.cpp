#include "ImageManager.h"
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;

HBITMAP ImageManager::LoadFromFile(const wstring& path, wstring& erreur) {
    HBITMAP hbm = (HBITMAP)LoadImageW(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (!hbm) {
        erreur = L"Échec du chargement de l'image.";
    }
    return hbm;
}

bool ImageManager::SaveToFile(HBITMAP hBitmap, const wstring& path, wstring& erreur) {
    if (!hBitmap) { 
        erreur = L"Aucun bitmap à sauvegarder"; 
        return false; 
    }

    BITMAP bmp = {};
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) { 
        erreur = L"GetObject échoué"; 
        return false; 
    }

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
    vector<BYTE> pixels(imageSize);

    BITMAPINFO bmi = {};
    bmi.bmiHeader = bi;

    HDC hdc = GetDC(NULL);
    if (GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc);
        erreur = L"GetDIBits échoué";

        return false;
    }
    ReleaseDC(NULL, hdc);

    // Préparer BITMAPFILEHEADER
    BITMAPFILEHEADER bfh = {};
    bfh.bfType = 0x4D42; // 'BM'
    // bfOffBits doit être aligné : sizeof(BITMAPFILEHEADER) + size of BITMAPINFOHEADER
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = (DWORD)(bfh.bfOffBits + imageSize);

    // écrire le fichier
    ofstream ofs(path, ios::binary);
    if (!ofs) { 
        erreur = L"Impossible d'ouvrir le fichier en écriture"; 
        return false; 
    }
    ofs.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
    ofs.write(reinterpret_cast<const char*>(&bi), sizeof(bi));
    ofs.write(reinterpret_cast<const char*>(pixels.data()), imageSize);
    ofs.close();

    return true;
}

void ImageManager::AddTitle(HDC hdcTitle, HWND hwnd, PAINTSTRUCT psTitle, const char* title, SIZE sizeTitle, int xTitle, int yTitle) {
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
