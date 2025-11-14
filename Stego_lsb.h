#pragma once
#include <string>
#include <vector>
#include <cstdint>


class Stego_lsb
{
	// Retourne true si l'encodage a réussi
	bool EmbedLSB(unsigned char* pixels, int width, int height, int stride, const std::string& message, std::string& out_err);

	// Retourne true si extraction OK ; extrait le message dans message_out
	bool ExtractLSB(const unsigned char* pixels, int width, int height, int stride, std::string& message_out, std::string& out_err);
};

