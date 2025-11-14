#include "Stego_lsb.h"
#include <cstring>
#include <cmath>

static const uint8_t MAGIC_BYTES[4] = { 'S','T','E','G' }; // "STEG"

static inline void WriteBitToByteLSB(unsigned char& byte, uint8_t bit) {
    byte = (byte & 0xFE) | (bit & 0x1); // remplace LSB
}
static inline uint8_t ReadBitFromByteLSB(unsigned char byte) {
    return (byte & 0x1);
}

// Convertit un uint32 en 4 octets little-endian dans dest[offset..offset+3]
static void WriteUint32LE(std::vector<uint8_t>& dest, uint32_t v, size_t offset) {
    dest[offset + 0] = (uint8_t)(v & 0xFF);
    dest[offset + 1] = (uint8_t)((v >> 8) & 0xFF);
    dest[offset + 2] = (uint8_t)((v >> 16) & 0xFF);
    dest[offset + 3] = (uint8_t)((v >> 24) & 0xFF);
}

static uint32_t ReadUint32LE(const std::vector<uint8_t>& src, size_t offset) {
    return (uint32_t)src[offset] | ((uint32_t)src[offset + 1] << 8) | ((uint32_t)src[offset + 2] << 16) | ((uint32_t)src[offset + 3] << 24);
}

bool Stego_lsb::EmbedLSB(unsigned char* pixels, int width, int height, int stride, const std::string& message, std::string& out_err) {
    if (!pixels) { out_err = "pixels == nullptr"; return false; }
    if (width <= 0 || height <= 0) { out_err = "dimensions invalides"; return false; }

    // Préparer payload = MAGIC (4) + length (4) + message bytes
    uint32_t msg_len = (uint32_t)message.size();
    std::vector<uint8_t> payload;
    payload.resize(8 + msg_len);
    // MAGIC
    memcpy(payload.data(), MAGIC_BYTES, 4);
    // longueur little-endian
    WriteUint32LE(payload, msg_len, 4);
    // message
    if (msg_len) memcpy(payload.data() + 8, message.data(), msg_len);

    uint64_t needed_bits = (uint64_t)payload.size() * 8ULL;
    uint64_t capacity_bits = (uint64_t)width * (uint64_t)height * 3ULL; // 3 bits/pixel

    if (needed_bits > capacity_bits) {
        out_err = "Image trop petite pour contenir le message. Capacité (bits): " + std::to_string(capacity_bits) +
            ", requis: " + std::to_string(needed_bits);
        return false;
    }

    // Parcours pixels et écrit bits
    // On parcourt les lignes de bas en haut si image DIB bottom-up ; ici on n'assume pas bottom/up,
    // car stride et buffer fournis par CreateDIBSection sont accessibles directement dans l'ordre mémoire.
    uint64_t bit_idx = 0;
    uint64_t total_bits = needed_bits;

    for (int y = 0; y < height && bit_idx < total_bits; ++y) {
        unsigned char* row = pixels + (size_t)y * (size_t)stride;
        for (int x = 0; x < width && bit_idx < total_bits; ++x) {
            unsigned char* px = row + x * 4; // BGRA
            // canaux B, G, R (ne pas toucher A)
            for (int chan = 0; chan < 3 && bit_idx < total_bits; ++chan) {
                // payload bit
                uint64_t byteIndex = bit_idx / 8;
                uint8_t bitInByte = 7 - (bit_idx % 8); // écrire MSB first of each payload byte
                uint8_t bit = (payload[byteIndex] >> bitInByte) & 0x1;
                WriteBitToByteLSB(px[chan], bit);
                ++bit_idx;
            }
        }
    }

    // terminé
    return true;
}

bool Stego_lsb::ExtractLSB(const unsigned char* pixels, int width, int height, int stride, std::string& message_out, std::string& out_err) {
    message_out.clear();
    if (!pixels) { out_err = "pixels == nullptr"; return false; }
    if (width <= 0 || height <= 0) { out_err = "dimensions invalides"; return false; }

    uint64_t capacity_bits = (uint64_t)width * (uint64_t)height * 3ULL;
    if (capacity_bits < 64) { out_err = "Image trop petite pour contenir header."; return false; } // besoin au moins MAGIC+len

    // Nous allons d'abord extraire 8 octets (64 bits) => MAGIC + length
    std::vector<uint8_t> header_bytes(8, 0);
    uint64_t bit_idx = 0;
    for (int y = 0; y < height && bit_idx < 64; ++y) {
        const unsigned char* row = pixels + (size_t)y * (size_t)stride;
        for (int x = 0; x < width && bit_idx < 64; ++x) {
            const unsigned char* px = row + x * 4;
            for (int chan = 0; chan < 3 && bit_idx < 64; ++chan) {
                uint64_t byteIndex = bit_idx / 8;
                uint8_t bitPos = 7 - (bit_idx % 8);
                uint8_t bit = ReadBitFromByteLSB(px[chan]);
                header_bytes[byteIndex] |= (bit << bitPos);
                ++bit_idx;
            }
        }
    }

    // Vérifier MAGIC
    if (memcmp(header_bytes.data(), MAGIC_BYTES, 4) != 0) {
        out_err = "MAGIC introuvable. L'image ne contient probablement pas de message steganographié.";
        return false;
    }

    uint32_t msg_len = ReadUint32LE(header_bytes, 4);
    uint64_t needed_bits = (uint64_t)msg_len * 8ULL + 64ULL;
    if (needed_bits > capacity_bits) {
        out_err = "Déclaration de longueur invalide ou image trop petite. Déclaré: " + std::to_string(msg_len) +
            " octets, capacité (bits): " + std::to_string(capacity_bits);
        return false;
    }

    // Extraire message
    std::vector<uint8_t> msgbuf(msg_len, 0);
    bit_idx = 64; // on reprend après header
    for (int y = 0; y < height && bit_idx < needed_bits; ++y) {
        const unsigned char* row = pixels + (size_t)y * (size_t)stride;
        for (int x = 0; x < width && bit_idx < needed_bits; ++x) {
            const unsigned char* px = row + x * 4;
            for (int chan = 0; chan < 3 && bit_idx < needed_bits; ++chan) {
                uint64_t bitPosOverall = bit_idx - 64;
                uint64_t byteIndex = bitPosOverall / 8;
                uint8_t bitPos = 7 - (bitPosOverall % 8);
                uint8_t bit = ReadBitFromByteLSB(px[chan]);
                msgbuf[byteIndex] |= (bit << bitPos);
                ++bit_idx;
            }
        }
    }

    message_out.assign((char*)msgbuf.data(), msgbuf.size());
    return true;
}