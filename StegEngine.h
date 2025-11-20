//#pragma once
//#include <windows.h>
//#include <vector>
//#include <bitset>
//
//class StegEngine
//{
//private:
//    BITMAP bmp;
//    int width = bmp.bmWidth;
//    int height = bmp.bmHeight;
//    int bpp = bmp.bmBitsPixel;
//    wchar_t buffer[1024] = {};
//public:
//	void EmbedLSB(HBITMAP hBitmap) {
//
//        // Récupération des dimensions de l'image
//        GetObject(hBitmap, sizeof(BITMAP), &bmp);
//
//        // Création d'un HDC pour manipuler l'image
//        HDC hdcMem = CreateCompatibleDC(NULL);
//        SelectObject(hdcMem, hBitmap);
//
//        // Préparation du bitmapinfo pour récupérer les pixels correctement
//        BITMAPINFO bmi = {};
//        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//        bmi.bmiHeader.biWidth = bmp.bmWidth;
//        bmi.bmiHeader.biHeight = bmp.bmHeight;
//        bmi.bmiHeader.biPlanes = 1;
//        bmi.bmiHeader.biBitCount = bmp.bmBitsPixel;
//        bmi.bmiHeader.biCompression = BI_RGB;
//
//        // "+ 31 / 32" --> Pour faire un arrondi au supérieur dans le cas où j'ai un nombre à virgule.. * 4 (octets) c'est pour la taille d'une ligne
//        int rowSize = ((bmp.bmWidth * bmp.bmBitsPixel + 31) / 32) * 4;
//        int imageSize = rowSize * bmp.bmHeight;
//        BYTE* pixels = new BYTE[imageSize];
//
//        GetDIBits(hdcMem, hBitmap, 0, bmp.bmHeight, pixels, &bmi, DIB_RGB_COLORS);
//
//        // --------------------------------------------- //
//
//        // Transforme le message en code binaire
//        std::vector<bool> bits;
//        for (wchar_t c : buffer)
//        {
//            std::bitset<8> b(c);  // b = bits du caractère c
//            for (int i = 0; i < 8; ++i)
//            {
//                bits.push_back(b[i]);  // on stocke chaque bit dans "bits"
//            }
//        }
//
//        size_t bitIndex = 0;
//        for (int y = 0; y < height && bitIndex < bits.size(); ++y)
//        {
//            for (int x = 0; x < width && bitIndex < bits.size(); ++x)
//            {
//                int idx = (height - 1 - y) * rowSize + x * 3;
//
//                // le LSB du bleu du pixel
//                pixels[idx] &= 0xFE; // On supprime le LSB
//                pixels[idx] |= bits[bitIndex++]; // On le remplace
//                if (bitIndex >= bits.size()) break; // Si on a finit d'écrire le message, on termine la boucle.
//
//                // le LSB du vert du pixel
//                pixels[idx + 1] &= 0xFE;
//                pixels[idx + 1] |= bits[bitIndex++];
//                if (bitIndex >= bits.size()) break;
//
//                // le LSB du rouge du pixel
//                pixels[idx + 2] &= 0xFE;
//                pixels[idx + 2] |= bits[bitIndex++];
//            }
//        }
//
//        SetDIBits(hdcMem, hBitmap, 0, bmp.bmHeight, pixels, &bmi, DIB_RGB_COLORS);
//	}
//
//	void ExtractLSB() {
//		// Méthode qui extrait un message codé
//	}
//};






#pragma once
#include <windows.h>
#include <string>

class StegEngine {
public:
    // Renvoie true si succès
    bool EmbedLSB(HBITMAP hBitmap, const std::wstring& message, std::wstring& err);
    // Renvoie true si message trouvé (et le met dans messageOut)
    bool ExtractLSB(HBITMAP hBitmap, std::wstring& messageOut, std::wstring& err);
private:
    static const uint32_t MAGIC = 0x47454553; // 'S' 'T' 'E' 'G' little-endian? we'll compare bytes explicitly
    // helpers
    std::string WideToUtf8(const std::wstring& w);
    std::wstring Utf8ToWide(const std::string& s);
};
