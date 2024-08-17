/*
 * File: DatabaseManager.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file defines the DatabaseManager class, which handles all database operations
 * for the Bid Management System. It provides an interface for CRUD operations on bids
 * and users, as well as additional functionality like CSV import and MFA management.
 *
 * Dependencies:
 * - sqlite3 for database operations
 * - CSVparser for CSV file parsing
 * - LinkedList for in-memory bid storage
 *
 */
#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "Bid.h"
#include "User.h"
#include "CSVparser.h"
#include "LinkedList.h"

class DatabaseManager {
private:
    sqlite3* db;  // SQLite database connection
    LinkedList bidList;  // In-memory storage for bids

    // Load bids from the database into memory
    void loadBidsIntoMemory();

public:
    DatabaseManager();
    ~DatabaseManager();

    // Initialize the database connection and tables
    void init();

    // CRUD operations for bids
    void addBid(const Bid& bid);
    Bid getBid(const std::string& bidId);
    std::vector<Bid> getAllBids();
    void updateBid(const Bid& bid);
    void deleteBid(const std::string& bidId);

    // User management
    void addUser(const User& user);
    User getUser(const std::string& username);
    bool validateUser(const std::string& username, const std::string& password);

    // CSV import functionalit
    void importFromCSV(const std::string& filename);

    // Sorting and searching
    std::vector<Bid> getSortedBids(bool (*comparator)(const Bid&, const Bid&));
    Bid binarySearchBid(const std::string& auctionId);

    // MFA management
    void enableMFA(const std::string& username, const std::string& totpSecret);
    bool isMFAEnabled(const std::string& username);
    std::string getTOTPSecret(const std::string& username);
};
