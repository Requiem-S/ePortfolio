/*
 * File: TOTP.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file defines the TOTP (Time-based One-Time Password) class, which provides
 * functionality for generating and verifying TOTP codes used in Multi-Factor Authentication.
 *
 * Dependencies:
 * - OpenSSL for cryptographic operations
 *
 */

#pragma once
#include <string>
#include <ctime>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <openssl/hmac.h>
#include <openssl/sha.h>

class TOTP {
public:
    // Generate a random secret key for TOTP
    static std::string generateSecret(int length = 32) {
        const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 31);

        std::string secret;
        for (int i = 0; i < length; ++i) {
            secret += charset[dis(gen)];
        }
        return secret;
    }

    // Generate a TOTP code based on the secret and current time
    static std::string generateTOTP(const std::string& secret, long timeStep = 30) {
        unsigned char hash[20];
        long T = std::time(nullptr) / timeStep;
        uint64_t stepT = htobe64(T);

        std::vector<unsigned char> key = base32Decode(secret);

        unsigned int md_len;
        HMAC(EVP_sha1(), key.data(), key.size(), (unsigned char*)&stepT, sizeof(stepT), hash, &md_len);

        int offset = hash[19] & 0xf;
        int binary =
            ((hash[offset] & 0x7f) << 24) |
            ((hash[offset + 1] & 0xff) << 16) |
            ((hash[offset + 2] & 0xff) << 8) |
            (hash[offset + 3] & 0xff);

        int otp = binary % 1000000;

        std::stringstream ss;
        ss << std::setw(6) << std::setfill('0') << otp;
        return ss.str();
    }

    // Verify a given TOTP code against the secret
    static bool verifyTOTP(const std::string& secret, const std::string& token, long timeStep = 30) {
        std::string generatedToken = generateTOTP(secret, timeStep);
        return generatedToken == token;
    }

private:
    // Decode a base32 encoded string
    static std::vector<unsigned char> base32Decode(const std::string& input) {
        const std::string base32Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        std::vector<unsigned char> output;
        int buffer = 0;
        int bitsLeft = 0;

        for (char c : input) {
            buffer <<= 5;
            buffer |= base32Chars.find(c);
            bitsLeft += 5;
            if (bitsLeft >= 8) {
                bitsLeft -= 8;
                output.push_back(buffer >> bitsLeft);
                buffer &= (1 << bitsLeft) - 1;
            }
        }

        return output;
    }

    // Convert host byte order to big endian
    static uint64_t htobe64(uint64_t host_64bits) {
        static const int num = 42;
        if (*(reinterpret_cast<const char*>(&num)) == num) {
            const uint32_t high_part = static_cast<uint32_t>(host_64bits >> 32);
            const uint32_t low_part = static_cast<uint32_t>(host_64bits & 0xFFFFFFFFLL);

            return (static_cast<uint64_t>(htonl(low_part)) << 32) | htonl(high_part);
        }
        else {
            return host_64bits;
        }
    }
};
