#include "StegEngine.h"
#include <vector>
#include <cstring>

using namespace std;

// Fonction qui convertit un texte UTF-16 en UTF-8
string StegEngine::WideToUtf8(const wstring& texte) {
    if (texte.empty()) {
        return {};
    }

    int taille = WideCharToMultiByte(CP_UTF8, 0, texte.data(), (int)texte.size(), NULL, 0, NULL, NULL);
    string texteConverti(taille, 0);
    WideCharToMultiByte(CP_UTF8, 0, texte.data(), (int)texte.size(), &texteConverti[0], taille, NULL, NULL);
    return texteConverti;
}

// Fonction qui convertit un texte UTF-8 en UTF-16
wstring StegEngine::Utf8ToWide(const string& texte) {
    if (texte.empty()) {
        return {};
    }

    int taille = MultiByteToWideChar(CP_UTF8, 0, texte.data(), (int)texte.size(), NULL, 0);
    wstring texteConverti(taille, 0);
    MultiByteToWideChar(CP_UTF8, 0, texte.data(), (int)texte.size(), &texteConverti[0], taille);
    return texteConverti;
}

bool StegEngine::EmbedLSB(HBITMAP hBitmap, const wstring& message, wstring& erreur) {
    if (!hBitmap) { 
        erreur = L"Erreur : Aucun bitmap n'est charge."; 
        return false;
    }

    BITMAP bmp;
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) { 
        erreur = L"Erreur : Impossible d'obtenir le BITMAP"; 
        return false; 
    }

    // On va travailler en 24bpp pour simplifier. Si bmp.bmBitsPixel != 24 && != 32 -> erreur pour l'instant
    if (bmp.bmBitsPixel != 24 && bmp.bmBitsPixel != 32) {
        erreur = L"Erreur : Format d'image non supporte. Utiliser BMP 24bpp ou 32bpp.";
        return false;
    }

    // Préparer BITMAPINFO pour récupérer les pixels en format 24 bits
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = bmp.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; // on récupère en 24 bits
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    HDC hdc = GetDC(NULL);
    int rowSize = ((bmp.bmWidth * 24 + 31) / 32) * 4;
    int imageSize = rowSize * bmp.bmHeight;
    vector<BYTE> pixels(imageSize);

    if (GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc);
        erreur = L"Erreur : GetDIBits a echoue";
        return false;
    }
    ReleaseDC(NULL, hdc);

    // Construire le message caché : MAGIC(4 bytes) + longueur(uint32LE) + message bytes (UTF8)
    string messageCache;

    // MAGIC en ASCII 'S','T','E','G' pour vérifier qu'un message est bien présent
    messageCache.push_back('S'); messageCache.push_back('T'); messageCache.push_back('E'); messageCache.push_back('G');

    string msgUtf8 = WideToUtf8(message);
    uint32_t len = (uint32_t)msgUtf8.size();

    messageCache.push_back((char)(len & 0xFF));
    messageCache.push_back((char)((len >> 8) & 0xFF));
    messageCache.push_back((char)((len >> 16) & 0xFF));
    messageCache.push_back((char)((len >> 24) & 0xFF));

    // ajoute les bits des messages
    messageCache.append(msgUtf8);

    // capacité en bits = width * height * 3 (channels) car 3 LSBs par pixel (BGR)
    uint64_t capacityBits = (uint64_t)bmp.bmWidth * (uint64_t)bmp.bmHeight * 3;
    uint64_t neededBits = (uint64_t)messageCache.size() * 8;

    if (neededBits > capacityBits) {
        erreur = L"Erreur : Le message est trop grand pour l'image, veuillez reessayer.";
        return false;
    }

    // Écriture du message dans les LSB de chaque pixel (BGR)
    size_t bitIndex = 0;
    for (int y = 0; y < bmp.bmHeight && bitIndex < neededBits; y++) {
        for (int x = 0; x < bmp.bmWidth && bitIndex < neededBits; x++) {

            int idx = (bmp.bmHeight - 1 - y) * rowSize + x * 3; // (bmp.bmHeight - 1 - y) -> les pixels sont à l'envers.
            for (int c = 0; c < 3 && bitIndex < neededBits; ++c) {

                size_t byteIndex = bitIndex / 8;
                int bitInByte = bitIndex % 8;

                BYTE bit = (messageCache[byteIndex] >> bitInByte) & 0x1;
                pixels[idx + c] = (pixels[idx + c] & 0xFE) | bit;
                ++bitIndex;
            }
        }
    }

    // Écriture dans le bitmap
    HDC hdc2 = GetDC(NULL);
    if (SetDIBits(hdc2, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc2);
        erreur = L"Erreur : SetDIBits a une erreur.";
        return false;
    }
    else {
        ReleaseDC(NULL, hdc2);
        return true;
    }
}

bool StegEngine::ExtractLSB(HBITMAP hBitmap, wstring& messageOut, wstring& err) {
    messageOut.clear();
    if (!hBitmap) { err = L"Aucun bitmap charge"; return false; }

    BITMAP bmp;
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) { err = L"Impossible d'obtenir le BITMAP"; return false; }

    if (bmp.bmBitsPixel != 24 && bmp.bmBitsPixel != 32) {
        err = L"Format d'image non supporte pour extraction.";
        return false;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = bmp.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    int rowSize = ((bmp.bmWidth * 24 + 31) / 32) * 4;
    int imageSize = rowSize * bmp.bmHeight;
    vector<BYTE> pixels(imageSize);
    HDC hdc = GetDC(NULL);
    if (GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc);
        err = L"GetDIBits a echoue (extraction)";
        return false;
    }
    ReleaseDC(NULL, hdc);

    // Lire les premiers (4 + 4) bytes pour le header
    auto read_bit = [&](uint64_t bitPos)->BYTE {
        uint64_t pixelIndex = bitPos / 3; // chaque pixel donne 3 bits
        uint64_t channel = bitPos % 3; // 0=B,1=G,2=R
        uint64_t y = pixelIndex / bmp.bmWidth;
        uint64_t x = pixelIndex % bmp.bmWidth;

        int idx = (bmp.bmHeight - 1 - (int)y) * rowSize + (int)x * 3;
        BYTE b = (pixels[idx + (int)channel] & 0x1);
        return b;

        };

    // on lit d'abord 8*8 = 64 bits ? Non : header = 8 bytes => 64 bits
    vector<BYTE> headerBytes(8, 0);
    for (uint64_t i = 0; i < 8; ++i) {
        BYTE val = 0;
        for (int bit = 0; bit < 8; ++bit) {
            BYTE bitv = read_bit(i * 8 + bit);
            val |= (bitv << bit);
        }
        headerBytes[i] = val;
    }

    if (!(headerBytes[0] == 'S' && headerBytes[1] == 'T' && headerBytes[2] == 'E' && headerBytes[3] == 'G')) {
        err = L"Aucun message cache (MAGIC manquant)";
        return false;
    }

    uint32_t len = (uint32_t)((uint8_t)headerBytes[4] | ((uint8_t)headerBytes[5] << 8) | ((uint8_t)headerBytes[6] << 16) | ((uint8_t)headerBytes[7] << 24));
    uint64_t capacityBits = (uint64_t)bmp.bmWidth * (uint64_t)bmp.bmHeight * 3;
    uint64_t requiredBits = (uint64_t)(8 + 4 + len) * 8;
    uint64_t availablePayloadBits = capacityBits - 64;

    if ((uint64_t)len * 8 > availablePayloadBits) {
        err = L"Entete incoherent (longueur trop grande)";
        return false;
    }

    vector<char> payload(len, 0);
    uint64_t bitStart = 8 * 8;
    for (uint32_t i = 0; i < len; ++i) {
        unsigned char val = 0;
        for (int bit = 0; bit < 8; ++bit) {
            uint64_t bpos = bitStart + (uint64_t)i * 8 + bit;
            BYTE bitv = read_bit(bpos);
            val |= (bitv << bit);
        }
        payload[i] = (char)val;
    }

    string s(payload.begin(), payload.end());
    messageOut = Utf8ToWide(s);
    return true;
}
