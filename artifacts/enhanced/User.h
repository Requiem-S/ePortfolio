/*
 * File: User.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file defines the User structure, which represents a user account in the
 * Bid Management System. It includes fields for authentication and MFA.
 *
 * Dependencies: None
 *
 */

#pragma once
#include <string>

struct User {
    std::string username;
    std::string passwordHash;
    std::string totpSecret;  // Secret key for Time-based One-Time Password (TOTP)
    bool mfaEnabled;         // Flag to indicate if MFA is enabled

    User() : mfaEnabled(false) {}  // Default constructor
};