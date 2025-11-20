#pragma once
#include <windows.h>
#include <string>

class StegEngine {
public:
    // Renvoie true si tout a fonctionner
    bool EmbedLSB(HBITMAP hBitmap, const std::wstring& message, std::wstring& erreur);
    // Renvoie true si message trouvé (et le met dans messageOut)
    bool ExtractLSB(HBITMAP hBitmap, std::wstring& messageOut, std::wstring& erreur);
private:
    static const uint32_t MAGIC = 0x47454553; // 'S' 'T' 'E' 'G'
    std::string WideToUtf8(const std::wstring& texteConverti);
    std::wstring Utf8ToWide(const std::string& texteConverti);
};
