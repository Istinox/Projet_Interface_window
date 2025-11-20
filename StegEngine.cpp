#include "StegEngine.h"
#include <vector>
#include <cstring>
#include <stdexcept>

std::string StegEngine::WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string out(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &out[0], size, NULL, NULL);
    return out;
}

std::wstring StegEngine::Utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
    std::wstring out(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &out[0], size);
    return out;
}

bool StegEngine::EmbedLSB(HBITMAP hBitmap, const std::wstring& message, std::wstring& err) {
    if (!hBitmap) { err = L"Aucun bitmap charge"; return false; }

    BITMAP bmp;
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) { err = L"Impossible d'obtenir le BITMAP"; return false; }

    // On va travailler en 24bpp pour simplicité. Si bmp.bmBitsPixel != 24 && != 32 -> erreur pour l'instant
    if (bmp.bmBitsPixel != 24 && bmp.bmBitsPixel != 32) {
        err = L"Format d'image non supporte. Utiliser BMP 24bpp ou 32bpp.";
        return false;
    }

    // Préparer BITMAPINFO pour récupérer les pixels en format 24bpp (force)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = bmp.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; // on récupère en 24 bpp
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    HDC hdc = GetDC(NULL);
    int rowSize = ((bmp.bmWidth * 24 + 31) / 32) * 4;
    int imageSize = rowSize * bmp.bmHeight;
    std::vector<BYTE> pixels(imageSize);
    if (GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc);
        err = L"GetDIBits a echoue";
        return false;
    }
    ReleaseDC(NULL, hdc);

    // Construire payload : MAGIC(4 bytes) + length(uint32LE) + message bytes (UTF8)
    std::string payload;
    // MAGIC as ASCII 'S','T','E','G'
    payload.push_back('S'); payload.push_back('T'); payload.push_back('E'); payload.push_back('G');

    std::string msgUtf8 = WideToUtf8(message);
    uint32_t len = (uint32_t)msgUtf8.size();
    // append length little-endian
    payload.push_back((char)(len & 0xFF));
    payload.push_back((char)((len >> 8) & 0xFF));
    payload.push_back((char)((len >> 16) & 0xFF));
    payload.push_back((char)((len >> 24) & 0xFF));
    // append message bytes
    payload.append(msgUtf8);

    // capacité en bits = width * height * 3 (channels) car 3 LSBs par pixel
    uint64_t capacityBits = (uint64_t)bmp.bmWidth * (uint64_t)bmp.bmHeight * 3;
    uint64_t neededBits = (uint64_t)payload.size() * 8;
    if (neededBits > capacityBits) {
        err = L"Message trop grand pour l'image.";
        return false;
    }

    // écrire bits dans LSBs (B,G,R order). Les pixels sont stockés bottom-up dans DIB (ligne inversée)
    size_t bitIndex = 0;
    for (int y = 0; y < bmp.bmHeight && bitIndex < neededBits; ++y) {
        for (int x = 0; x < bmp.bmWidth && bitIndex < neededBits; ++x) {
            int idx = (bmp.bmHeight - 1 - y) * rowSize + x * 3; // B G R
            for (int c = 0; c < 3 && bitIndex < neededBits; ++c) {
                size_t byteIndex = bitIndex / 8;
                int bitInByte = bitIndex % 8;
                BYTE bit = (payload[byteIndex] >> bitInByte) & 0x1;
                pixels[idx + c] = (pixels[idx + c] & 0xFE) | bit;
                ++bitIndex;
            }
        }
    }

    // écrire au bitmap
    HDC hdc2 = GetDC(NULL);
    if (SetDIBits(hdc2, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc2);
        err = L"SetDIBits a echoue";
        return false;
    }
    ReleaseDC(NULL, hdc2);
    return true;
}

bool StegEngine::ExtractLSB(HBITMAP hBitmap, std::wstring& messageOut, std::wstring& err) {
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
    std::vector<BYTE> pixels(imageSize);
    HDC hdc = GetDC(NULL);
    if (GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) == 0) {
        ReleaseDC(NULL, hdc);
        err = L"GetDIBits a echoue (extraction)";
        return false;
    }
    ReleaseDC(NULL, hdc);

    // Lire premiers (4 + 4) bytes pour header
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
    std::vector<BYTE> headerBytes(8, 0);
    for (uint64_t i = 0; i < 8; ++i) {
        BYTE val = 0;
        for (int bit = 0; bit < 8; ++bit) {
            BYTE bitv = read_bit(i * 8 + bit);
            val |= (bitv << bit);
        }
        headerBytes[i] = val;
    }

    // check MAGIC
    if (!(headerBytes[0] == 'S' && headerBytes[1] == 'T' && headerBytes[2] == 'E' && headerBytes[3] == 'G')) {
        err = L"Aucun message cache (MAGIC manquant)";
        return false;
    }

    uint32_t len = (uint32_t)((uint8_t)headerBytes[4] | ((uint8_t)headerBytes[5] << 8) | ((uint8_t)headerBytes[6] << 16) | ((uint8_t)headerBytes[7] << 24));
    // capacité check
    uint64_t capacityBits = (uint64_t)bmp.bmWidth * (uint64_t)bmp.bmHeight * 3;
    uint64_t requiredBits = (uint64_t)(8 + 4 + len) * 8; // header(8) + len(4) + payload
    // but we already used 8 bytes for header; simpler: ensure len reasonable
    uint64_t availablePayloadBits = capacityBits - 64; // after header bits
    if ((uint64_t)len * 8 > availablePayloadBits) {
        err = L"Entete incoherent (longueur trop grande)";
        return false;
    }

    std::vector<char> payload(len, 0);
    uint64_t bitStart = 8 * 8; // 64 bits already read
    for (uint32_t i = 0; i < len; ++i) {
        unsigned char val = 0;
        for (int bit = 0; bit < 8; ++bit) {
            uint64_t bpos = bitStart + (uint64_t)i * 8 + bit;
            BYTE bitv = read_bit(bpos);
            val |= (bitv << bit);
        }
        payload[i] = (char)val;
    }

    std::string s(payload.begin(), payload.end());
    messageOut = Utf8ToWide(s);
    return true;
}
