/*
 * File: DatabaseManager.cpp
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file implements the DatabaseManager class, handling all database operations
 * for the Bid Management System. It includes CRUD operations for bids and users,
 * CSV import functionality, and MFA management.
 *
 * Dependencies:
 * - sqlite3 for database operations
 * - CSVparser for CSV file parsing
 * - LinkedList for in-memory bid storage
 * - OpenSSL for password hashing
 *
 */

#include "DatabaseManager.h"
#include "Utils.h"
#include <stdexcept>
#include <vector>
#include <cstring>
#include <openssl/sha.h>

DatabaseManager::DatabaseManager() : db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

void DatabaseManager::init() {
    // Open the SQLite database
    int rc = sqlite3_open("bids.db", &db);
    if (rc) {
        throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(db)));
    }

    // SQL to create the bids table
    const char* sql = "CREATE TABLE IF NOT EXISTS bids ("
        "auction_title TEXT,"
        "auction_id TEXT PRIMARY KEY,"
        "department TEXT,"
        "close_date TEXT,"
        "winning_bid REAL,"
        "cc_fee REAL,"
        "fee_percent REAL,"
        "auction_fee_subtotal REAL,"
        "auction_fee_total REAL,"
        "pay_status TEXT,"
        "paid_date TEXT,"
        "asset_number TEXT,"
        "inventory_id TEXT,"
        "decal_vehicle_id TEXT,"
        "vtr_number TEXT,"
        "receipt_number TEXT,"
        "cap REAL,"
        "expenses REAL,"
        "net_sales REAL,"
        "fund TEXT,"
        "business_unit TEXT"
        ");";

    // SQL to create the users table
    const char* sql_users = "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY,"
        "password_hash TEXT,"
        "totp_secret TEXT,"
        "mfa_enabled INTEGER DEFAULT 0"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::string error = "SQL error: " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }

    // Load existing bids into the LinkedList
    loadBidsIntoMemory();
}

// Load bids from the database into memory
void DatabaseManager::loadBidsIntoMemory() {
    const char* sql = "SELECT * FROM bids;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Bid bid;
        // Populate bid object from database row
        bid.auctionTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        bid.auctionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bid.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        bid.closeDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        bid.winningBid = sqlite3_column_double(stmt, 4);
        bid.ccFee = sqlite3_column_double(stmt, 5);
        bid.feePercent = sqlite3_column_double(stmt, 6);
        bid.auctionFeeSubtotal = sqlite3_column_double(stmt, 7);
        bid.auctionFeeTotal = sqlite3_column_double(stmt, 8);
        bid.payStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        bid.paidDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        bid.assetNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        bid.inventoryId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        bid.decalVehicleId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        bid.vtrNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
        bid.receiptNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));
        bid.cap = sqlite3_column_double(stmt, 16);
        bid.expenses = sqlite3_column_double(stmt, 17);
        bid.netSales = sqlite3_column_double(stmt, 18);
        bid.fund = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 19));
        bid.businessUnit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 20));

        bidList.Append(bid);
    }

    sqlite3_finalize(stmt);
}

// Add a new bid to the database and in-memory list
void DatabaseManager::addBid(const Bid& bid) {
    const char* sql = "INSERT INTO bids (auction_title, auction_id, department, close_date, winning_bid, cc_fee, fee_percent, auction_fee_subtotal, auction_fee_total, pay_status, paid_date, asset_number, inventory_id, decal_vehicle_id, vtr_number, receipt_number, cap, expenses, net_sales, fund, business_unit) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    // Bind values to the prepared statement
    sqlite3_bind_text(stmt, 1, bid.auctionTitle.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bid.auctionId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, bid.department.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, bid.closeDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, bid.winningBid);
    sqlite3_bind_double(stmt, 6, bid.ccFee);
    sqlite3_bind_double(stmt, 7, bid.feePercent);
    sqlite3_bind_double(stmt, 8, bid.auctionFeeSubtotal);
    sqlite3_bind_double(stmt, 9, bid.auctionFeeTotal);
    sqlite3_bind_text(stmt, 10, bid.payStatus.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, bid.paidDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, bid.assetNumber.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 13, bid.inventoryId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 14, bid.decalVehicleId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 15, bid.vtrNumber.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 16, bid.receiptNumber.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 17, bid.cap);
    sqlite3_bind_double(stmt, 18, bid.expenses);
    sqlite3_bind_double(stmt, 19, bid.netSales);
    sqlite3_bind_text(stmt, 20, bid.fund.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 21, bid.businessUnit.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to insert bid: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    // Also add to in-memory list
    bidList.Append(bid);
}

// Retrieve a bid by its auction ID
Bid DatabaseManager::getBid(const std::string& auctionId) {
    // First, try to find the bid in the in-memory list
    Bid bid = bidList.Search(auctionId);
    if (!bid.auctionId.empty()) {
        return bid;
    }

    // If not found in memory, search in the database
    const char* sql = "SELECT * FROM bids WHERE auction_id = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, auctionId.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Bid not found");
    }

    // Populate bid object from database row
    bid.auctionTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    bid.auctionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    bid.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    bid.closeDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    bid.winningBid = sqlite3_column_double(stmt, 4);
    bid.ccFee = sqlite3_column_double(stmt, 5);
    bid.feePercent = sqlite3_column_double(stmt, 6);
    bid.auctionFeeSubtotal = sqlite3_column_double(stmt, 7);
    bid.auctionFeeTotal = sqlite3_column_double(stmt, 8);
    bid.payStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
    bid.paidDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
    bid.assetNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    bid.inventoryId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    bid.decalVehicleId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
    bid.vtrNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
    bid.receiptNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));
    bid.cap = sqlite3_column_double(stmt, 16);
    bid.expenses = sqlite3_column_double(stmt, 17);
    bid.netSales = sqlite3_column_double(stmt, 18);
    bid.fund = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 19));
    bid.businessUnit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 20));

    sqlite3_finalize(stmt);

    // Add to in-memory list for future quick access
    bidList.Append(bid);

    return bid;
}

// Get all bids from the in-memory list
std::vector<Bid> DatabaseManager::getAllBids() {
    return bidList.GetAllBids();
}

// Update an existing bid
void DatabaseManager::updateBid(const Bid& bid) {
    const char* sql = "UPDATE bids SET auction_title = ?, department = ?, close_date = ?, winning_bid = ?, cc_fee = ?, fee_percent = ?, auction_fee_subtotal = ?, auction_fee_total = ?, pay_status = ?, paid_date = ?, asset_number = ?, inventory_id = ?, decal_vehicle_id = ?, vtr_number = ?, receipt_number = ?, cap = ?, expenses = ?, net_sales = ?, fund = ?, business_unit = ? WHERE auction_id = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    // Bind values to the prepared statement
    sqlite3_bind_text(stmt, 1, bid.auctionTitle.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bid.department.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, bid.closeDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, bid.winningBid);
    sqlite3_bind_double(stmt, 5, bid.ccFee);
    sqlite3_bind_double(stmt, 6, bid.feePercent);
    sqlite3_bind_double(stmt, 7, bid.auctionFeeSubtotal);
    sqlite3_bind_double(stmt, 8, bid.auctionFeeTotal);
    sqlite3_bind_text(stmt, 9, bid.payStatus.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, bid.paidDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, bid.assetNumber.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, bid.inventoryId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 13, bid.decalVehicleId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 14, bid.vtrNumber.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 15, bid.receiptNumber.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 16, bid.cap);
    sqlite3_bind_double(stmt, 17, bid.expenses);
    sqlite3_bind_double(stmt, 18, bid.netSales);
    sqlite3_bind_text(stmt, 19, bid.fund.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 20, bid.businessUnit.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 21, bid.auctionId.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to update bid: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    // Update in-memory list
    bidList.Remove(bid.auctionId);
    bidList.Append(bid);
}

// Delete a bid by its auction ID
void DatabaseManager::deleteBid(const std::string& auctionId) {
    const char* sql = "DELETE FROM bids WHERE auction_id = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, auctionId.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to delete bid: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    // Remove from in-memory list
    bidList.Remove(auctionId);
}

// Add a new user to the database
void DatabaseManager::addUser(const User& user) {
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.passwordHash.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to insert user: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
}

// Retrieve a user by username
User DatabaseManager::getUser(const std::string& username) {
    const char* sql = "SELECT * FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("User not found");
    }

    User user;
    user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    user.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

    sqlite3_finalize(stmt);
    return user;
}

// Validate user credentials
bool DatabaseManager::validateUser(const std::string& username, const std::string& password) {
    try {
        User user = getUser(username);
        return user.passwordHash == hashPassword(password);
    }
    catch (const std::runtime_error&) {
        return false;
    }
}

// Import bids from a CSV file
void DatabaseManager::importFromCSV(const std::string& filename) {
    std::cout << "Attempting to import CSV from: " << filename << std::endl;
    try {
        csv::Parser parser(filename);
        std::cout << "Successfully opened CSV file. Row count: " << parser.rowCount() << std::endl;

        // Skip the header row
        for (unsigned int i = 1; i < parser.rowCount(); i++) {
            try {
                csv::Row& row = parser[i];

                Bid bid;
                // Populate bid object from CSV row
                bid.auctionTitle = row[0];
                bid.auctionId = row[1];
                bid.department = row[2];
                bid.closeDate = row[3];

                // Add error checking for numeric conversions
                try {
                    bid.winningBid = std::stod(row[4].empty() ? "0" : row[4].substr(row[4].find_first_not_of(" $")));
                    bid.ccFee = std::stod(row[5].empty() ? "0" : row[5].substr(row[5].find_first_not_of(" $")));
                    bid.feePercent = std::stod(row[6].empty() ? "0" : row[6]);
                    bid.auctionFeeSubtotal = std::stod(row[7].empty() ? "0" : row[7].substr(row[7].find_first_not_of(" $")));
                    bid.auctionFeeTotal = std::stod(row[8].empty() ? "0" : row[8].substr(row[8].find_first_not_of(" $")));
                }
                catch (const std::exception& e) {
                    std::cerr << "Error converting numeric values in row " << i << ": " << e.what() << std::endl;
                    continue; // Skip this row and move to the next
                }

                bid.payStatus = row[9];
                bid.paidDate = row[10];
                bid.assetNumber = row[11];
                bid.inventoryId = row[12];
                bid.decalVehicleId = row[13];
                bid.vtrNumber = row[14];
                bid.receiptNumber = row[15];

                // Add error checking for cap conversion
                try {
                    std::string capStr = row[16];
                    capStr.erase(std::remove_if(capStr.begin(), capStr.end(), [](char c) { return c == '$' || c == ',' || std::isspace(c); }), capStr.end());
                    bid.cap = std::stod(capStr.empty() ? "0" : capStr);
                }
                catch (const std::exception& e) {
                    std::cerr << "Error converting cap value in row " << i << ": " << e.what() << std::endl;
                    continue; // Skip this row and move to the next
                }

                // Add error checking for expenses and netSales conversions
                try {
                    bid.expenses = std::stod(row[17].empty() ? "0" : row[17].substr(row[17].find_first_not_of(" $")));
                    bid.netSales = std::stod(row[18].empty() ? "0" : row[18].substr(row[18].find_first_not_of(" $")));
                }
                catch (const std::exception& e) {
                    std::cerr << "Error converting expenses or netSales in row " << i << ": " << e.what() << std::endl;
                    continue; // Skip this row and move to the next
                }

                bid.fund = row[19];
                bid.businessUnit = row[20];

                addBid(bid);
                std::cout << "Imported bid: " << bid.auctionId << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing row " << i << ": " << e.what() << std::endl;
            }
        }
        std::cout << "CSV import completed successfully" << std::endl;
    }
    catch (csv::Error& e) {
        std::cerr << "CSV Parser error: " << e.what() << std::endl;
        throw std::runtime_error(std::string("CSV Parser error: ") + e.what());
    }
    catch (std::exception& e) {
        std::cerr << "Error during CSV import: " << e.what() << std::endl;
        throw;
    }
}

// New methods using LinkedList functionalities
// Get sorted bids using a custom comparator
std::vector<Bid> DatabaseManager::getSortedBids(bool (*comparator)(const Bid&, const Bid&)) {
    LinkedList sortedList = bidList; // Create a copy of the list
    sortedList.Sort(comparator);
    return sortedList.GetAllBids();
}

// Perform binary search on the bid list
Bid DatabaseManager::binarySearchBid(const std::string& auctionId) {
    return bidList.BinarySearch(auctionId);

}


// Enable Multi-Factor Authentication for a user
void DatabaseManager::enableMFA(const std::string& username, const std::string& totpSecret) {
    const char* sql = "UPDATE users SET totp_secret = ?, mfa_enabled = 1 WHERE username = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, totpSecret.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to enable MFA: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
}

// Check if MFA is enabled for a user
bool DatabaseManager::isMFAEnabled(const std::string& username) {
    const char* sql = "SELECT mfa_enabled FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("User not found");
    }

    bool mfaEnabled = sqlite3_column_int(stmt, 0) == 1;
    sqlite3_finalize(stmt);

    return mfaEnabled;
}

// Get the TOTP secret for a user
std::string DatabaseManager::getTOTPSecret(const std::string& username) {
    const char* sql = "SELECT totp_secret FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("User not found");
    }

    std::string totpSecret = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);

    return totpSecret;
}