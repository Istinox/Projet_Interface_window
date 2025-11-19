#pragma once
#include <windows.h>
#include <vector>
#include <bitset>

class StegEngine
{
private:
    BITMAP bmp;
    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int bpp = bmp.bmBitsPixel;

public:
	void EmbedLSB(HBITMAP hBitmap, wchar_t buffer[1024]) {

        // Récupération des dimensions de l'image
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        // Création d'un HDC pour manipuler l'image
        HDC hdcMem = CreateCompatibleDC(NULL);
        SelectObject(hdcMem, hBitmap);

        // Préparation du bitmapinfo pour récupérer les pixels correctement
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = bmp.bmWidth;
        bmi.bmiHeader.biHeight = bmp.bmHeight;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = bmp.bmBitsPixel;
        bmi.bmiHeader.biCompression = BI_RGB;

        // "+ 31 / 32" --> Pour faire un arrondi au supérieur dans le cas où j'ai un nombre à virgule.. * 4 (octets) c'est pour la taille d'une ligne
        int rowSize = ((bmp.bmWidth * bmp.bmBitsPixel + 31) / 32) * 4;
        int imageSize = rowSize * bmp.bmHeight;
        BYTE* pixels = new BYTE[imageSize];

        GetDIBits(hdcMem, hBitmap, 0, bmp.bmHeight, pixels, &bmi, DIB_RGB_COLORS);

        // --------------------------------------------- //

        // Transforme le message en code binaire
        std::vector<bool> bits;

        // Il faudrait plutôt prendre la taille du buffer plutôt que sa taille max..
        for (size_t i = 0; i < 1024; ++i) {
            std::bitset<8> b(buffer[i]);
            for (int j = 0; j < 8; ++j) {
                bits.push_back(b[j]);
            }
        }

        size_t bitIndex = 0;
        for (int y = 0; y < height && bitIndex < bits.size(); ++y)
        {
            for (int x = 0; x < width && bitIndex < bits.size(); ++x)
            {
                int idx = (height - 1 - y) * rowSize + x * 3;

                // le LSB du bleu du pixel
                pixels[idx] &= 0xFE; // On supprime le LSB
                pixels[idx] |= bits[bitIndex++]; // On le remplace
                if (bitIndex >= bits.size()) break; // Si on a finit d'écrire le message, on termine la boucle.

                // le LSB du vert du pixel
                pixels[idx + 1] &= 0xFE;
                pixels[idx + 1] |= bits[bitIndex++];
                if (bitIndex >= bits.size()) break;

                // le LSB du rouge du pixel
                pixels[idx + 2] &= 0xFE;
                pixels[idx + 2] |= bits[bitIndex++];
            }
        }

        SetDIBits(hdcMem, hBitmap, 0, bmp.bmHeight, pixels, &bmi, DIB_RGB_COLORS);
	}

	void ExtractLSB() {
		// Méthode qui extrait un message codé
	}
};

