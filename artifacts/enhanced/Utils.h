/*
 * File: Utils.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file contains utility functions used throughout the Bid Management System,
 * particularly for cryptographic operations like password hashing.
 *
 * Dependencies:
 * - OpenSSL for cryptographic functions
 *
 */
#pragma once
#include <string>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

 // Function to hash a password using SHA-256
inline std::string hashPassword(const std::string& password) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    md = EVP_sha256();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password.c_str(), password.length());
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    // Convert the hash to a hexadecimal string
    std::stringstream ss;
    for (unsigned int i = 0; i < md_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md_value[i]);
    }

    return ss.str();
}