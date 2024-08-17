/*
 * File: BidManagementServer.cpp
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file implements the main server functionality for the Bid Management System.
 * It sets up the HTTP server, defines API endpoints, and handles user authentication
 * and bid management operations.
 *
 * Dependencies:
 * - crow framework for HTTP server
 * - DatabaseManager for data persistence
 * - JWT for token-based authentication
 * 
 */

#include "crow.h"
#include "crow/middleware.h"
#include "DatabaseManager.h"
#include "Bid.h"
#include "User.h"
#include "Utils.h"
#include "TOTP.h"
#include <vector>
#include <stdexcept>
#include <jwt-cpp/jwt.h>


 // Function to validate the input JSON for a bid
bool validateBidInput(const crow::json::rvalue& json) {

    // Check if all required fields are present and have the correct types
    return json.has("auctionTitle") && json.has("auctionId") && json.has("department") &&
        json.has("closeDate") && json.has("winningBid") && json.has("ccFee") &&
        json.has("feePercent") && json.has("auctionFeeSubtotal") && json.has("auctionFeeTotal") &&
        json.has("payStatus") && json.has("paidDate") && json.has("assetNumber") &&
        json.has("inventoryId") && json.has("decalVehicleId") && json.has("vtrNumber") &&
        json.has("receiptNumber") && json.has("cap") && json.has("expenses") &&
        json.has("netSales") && json.has("fund") && json.has("businessUnit") &&
        json["auctionTitle"].t() == crow::json::type::String &&
        json["auctionId"].t() == crow::json::type::String &&
        json["department"].t() == crow::json::type::String &&
        json["closeDate"].t() == crow::json::type::String &&
        json["winningBid"].t() == crow::json::type::Number &&
        json["ccFee"].t() == crow::json::type::Number &&
        json["feePercent"].t() == crow::json::type::Number &&
        json["auctionFeeSubtotal"].t() == crow::json::type::Number &&
        json["auctionFeeTotal"].t() == crow::json::type::Number &&
        json["payStatus"].t() == crow::json::type::String &&
        json["paidDate"].t() == crow::json::type::String &&
        json["assetNumber"].t() == crow::json::type::String &&
        json["inventoryId"].t() == crow::json::type::String &&
        json["decalVehicleId"].t() == crow::json::type::String &&
        json["vtrNumber"].t() == crow::json::type::String &&
        json["receiptNumber"].t() == crow::json::type::String &&
        json["cap"].t() == crow::json::type::Number &&
        json["expenses"].t() == crow::json::type::Number &&
        json["netSales"].t() == crow::json::type::Number &&
        json["fund"].t() == crow::json::type::String &&
        json["businessUnit"].t() == crow::json::type::String;
}

// Function to create a JWT token for authenticated users
std::string createToken(const std::string& username) {
    // Create a JWT token with a 1-hour expiration time
    auto token = jwt::create()
        .set_issuer("auth0")
        .set_type("JWS")
        .set_payload_claim("username", jwt::claim(username))
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{ 1 })
        .sign(jwt::algorithm::hs256{ "secret" });
    return token;
}

// Function to verify the JWT token
bool verifyToken(const std::string& token) {
    try {
        // Decode and verify the token
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ "secret" })
            .with_issuer("auth0");
        verifier.verify(decoded);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

int main()
{
    crow::SimpleApp app;
    DatabaseManager dbManager;

    // Initialize the database
    try {
        dbManager.init();
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize database: " << e.what() << std::endl;
        return 1;
    }
    
    // Define routes and their handlers

    // Root route
    CROW_ROUTE(app, "/")([]() {
        return crow::response("Welcome to the Bid Management System!");
    });

    // User registration route
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        if (!x.has("username") || !x.has("password")) {
            return crow::response(400, "Missing username or password");
        }
        try {
            User user;
            user.username = x["username"].s();
            user.passwordHash = hashPassword(x["password"].s());
            dbManager.addUser(user);
            return crow::response(201, "User registered successfully");
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });


    // User login route
    CROW_ROUTE(app, "/login")
        .methods("POST"_method)
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        if (!x.has("username") || !x.has("password")) {
            return crow::response(400, "Missing username or password");
        }
        try {
            if (dbManager.validateUser(x["username"].s(), x["password"].s())) {
                std::string token = createToken(x["username"].s());
                std::cout << "Generated token: " << token << std::endl;  // Log the generated token
                return crow::response(200, token);
            }
            else {
                return crow::response(401, "Invalid username or password");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Middleware for token verification
    struct TokenVerifier {
        bool before(crow::request& req) {
            std::cout << "TokenVerifier: Checking authorization" << std::endl;
            auto auth_header = req.get_header_value("Authorization");
            std::cout << "TokenVerifier: Auth header: " << auth_header << std::endl;

            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
                std::cout << "TokenVerifier: Invalid Authorization header format" << std::endl;
                return false;
            }

            std::string token = auth_header.substr(7);
            std::cout << "TokenVerifier: Extracted token: " << token << std::endl;

            bool is_valid = verifyToken(token);
            std::cout << "TokenVerifier: Token validity: " << (is_valid ? "valid" : "invalid") << std::endl;

            return is_valid;
        }
    };

    // CSV import route
    CROW_ROUTE(app, "/import-csv")
        .methods("POST"_method)
        .middlewares<TokenVerifier>()
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x || !x.has("filename")) {
            return crow::response(400, "Invalid JSON or missing filename");
        }

        try {
            dbManager.importFromCSV(x["filename"].s());
            return crow::response(200, "CSV data imported successfully");
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Error importing CSV: ") + e.what());
        }
    });

    // Get all bids route
    CROW_ROUTE(app, "/bids")
        .methods("GET"_method)
        .middlewares<TokenVerifier>()
        ([&dbManager]() {
        try {
            std::vector<Bid> bids = dbManager.getAllBids();
            crow::json::wvalue response;
            for (size_t i = 0; i < bids.size(); i++) {
                response[i] = {
                    {"auctionTitle", bids[i].auctionTitle},
                    {"auctionId", bids[i].auctionId},
                    {"department", bids[i].department},
                    {"closeDate", bids[i].closeDate},
                    {"winningBid", bids[i].winningBid},
                    {"ccFee", bids[i].ccFee},
                    {"feePercent", bids[i].feePercent},
                    {"auctionFeeSubtotal", bids[i].auctionFeeSubtotal},
                    {"auctionFeeTotal", bids[i].auctionFeeTotal},
                    {"payStatus", bids[i].payStatus},
                    {"paidDate", bids[i].paidDate},
                    {"assetNumber", bids[i].assetNumber},
                    {"inventoryId", bids[i].inventoryId},
                    {"decalVehicleId", bids[i].decalVehicleId},
                    {"vtrNumber", bids[i].vtrNumber},
                    {"receiptNumber", bids[i].receiptNumber},
                    {"cap", bids[i].cap},
                    {"expenses", bids[i].expenses},
                    {"netSales", bids[i].netSales},
                    {"fund", bids[i].fund},
                    {"businessUnit", bids[i].businessUnit}
                };
            }
            return crow::response(response);
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Create new bid route
    CROW_ROUTE(app, "/bids")
        .methods("POST"_method)
        .middlewares<TokenVerifier>()
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        if (!validateBidInput(x)) {
            return crow::response(400, "Invalid bid data");
        }
        try {
            Bid bid;
            // Populate bid object from JSON
            bid.auctionTitle = x["auctionTitle"].s();
            bid.auctionId = x["auctionId"].s();
            bid.department = x["department"].s();
            bid.closeDate = x["closeDate"].s();
            bid.winningBid = x["winningBid"].d();
            bid.ccFee = x["ccFee"].d();
            bid.feePercent = x["feePercent"].d();
            bid.auctionFeeSubtotal = x["auctionFeeSubtotal"].d();
            bid.auctionFeeTotal = x["auctionFeeTotal"].d();
            bid.payStatus = x["payStatus"].s();
            bid.paidDate = x["paidDate"].s();
            bid.assetNumber = x["assetNumber"].s();
            bid.inventoryId = x["inventoryId"].s();
            bid.decalVehicleId = x["decalVehicleId"].s();
            bid.vtrNumber = x["vtrNumber"].s();
            bid.receiptNumber = x["receiptNumber"].s();
            bid.cap = x["cap"].d();
            bid.expenses = x["expenses"].d();
            bid.netSales = x["netSales"].d();
            bid.fund = x["fund"].s();
            bid.businessUnit = x["businessUnit"].s();
            dbManager.addBid(bid);
            return crow::response(201, "Bid created successfully");
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Get specific bid route
    CROW_ROUTE(app, "/bids/<string>")
        .methods("GET"_method)
        .middlewares<TokenVerifier>()
        ([&dbManager](const std::string& id) {
        try {
            Bid bid = dbManager.getBid(id);
            crow::json::wvalue response = {
                {"auctionTitle", bid.auctionTitle},
                {"auctionId", bid.auctionId},
                {"department", bid.department},
                {"closeDate", bid.closeDate},
                {"winningBid", bid.winningBid},
                {"ccFee", bid.ccFee},
                {"feePercent", bid.feePercent},
                {"auctionFeeSubtotal", bid.auctionFeeSubtotal},
                {"auctionFeeTotal", bid.auctionFeeTotal},
                {"payStatus", bid.payStatus},
                {"paidDate", bid.paidDate},
                {"assetNumber", bid.assetNumber},
                {"inventoryId", bid.inventoryId},
                {"decalVehicleId", bid.decalVehicleId},
                {"vtrNumber", bid.vtrNumber},
                {"receiptNumber", bid.receiptNumber},
                {"cap", bid.cap},
                {"expenses", bid.expenses},
                {"netSales", bid.netSales},
                {"fund", bid.fund},
                {"businessUnit", bid.businessUnit}
            };
            return crow::response(response);
        }
        catch (const std::runtime_error& e) {
            return crow::response(404, "Bid not found");
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Update bid route
    CROW_ROUTE(app, "/bids/<string>")
        .methods("PUT"_method)
        .middlewares<TokenVerifier>()
        ([&dbManager](const crow::request& req, const std::string& id) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        if (!validateBidInput(x)) {
            return crow::response(400, "Invalid bid data");
        }
        try {
            Bid bid;
            // Populate bid object from JSON
            bid.auctionTitle = x["auctionTitle"].s();
            bid.auctionId = id;
            bid.department = x["department"].s();
            bid.closeDate = x["closeDate"].s();
            bid.winningBid = x["winningBid"].d();
            bid.ccFee = x["ccFee"].d();
            bid.feePercent = x["feePercent"].d();
            bid.auctionFeeSubtotal = x["auctionFeeSubtotal"].d();
            bid.auctionFeeTotal = x["auctionFeeTotal"].d();
            bid.payStatus = x["payStatus"].s();
            bid.paidDate = x["paidDate"].s();
            bid.assetNumber = x["assetNumber"].s();
            bid.inventoryId = x["inventoryId"].s();
            bid.decalVehicleId = x["decalVehicleId"].s();
            bid.vtrNumber = x["vtrNumber"].s();
            bid.receiptNumber = x["receiptNumber"].s();
            bid.cap = x["cap"].d();
            bid.expenses = x["expenses"].d();
            bid.netSales = x["netSales"].d();
            bid.fund = x["fund"].s();
            bid.businessUnit = x["businessUnit"].s();
            dbManager.updateBid(bid);
            return crow::response(200, "Bid updated successfully");
        }
        catch (const std::runtime_error& e) {
            return crow::response(404, "Bid not found");
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Delete bid route
    CROW_ROUTE(app, "/bids/<string>")
        .methods("DELETE"_method)
        .middlewares<TokenVerifier>()
        ([&dbManager](const std::string& id) {
        try {
            dbManager.deleteBid(id);
            return crow::response(200, "Bid deleted successfully");
        }
        catch (const std::runtime_error& e) {
            return crow::response(404, "Bid not found");
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Enable MFA route
    CROW_ROUTE(app, "/enable-mfa")
        .methods("POST"_method)
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x || !x.has("username") || !x.has("password")) {
            return crow::response(400, "Invalid JSON or missing username/password");
        }

        try {
            // Validate user credentials before enabling MFA
            if (dbManager.validateUser(x["username"].s(), x["password"].s())) {
                std::string totpSecret = TOTP::generateSecret();
                dbManager.enableMFA(x["username"].s(), totpSecret);
                return crow::response(200, totpSecret);
            }
            else {
                return crow::response(401, "Invalid username or password");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Login route with MFA support
    CROW_ROUTE(app, "/login")
        .methods("POST"_method)
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x || !x.has("username") || !x.has("password")) {
            return crow::response(400, "Invalid JSON or missing username/password");
        }

        try {
            if (dbManager.validateUser(x["username"].s(), x["password"].s())) {
                if (dbManager.isMFAEnabled(x["username"].s())) {
                    return crow::response(200, "MFA required");
                }
                else {
                    std::string token = createToken(x["username"].s());
                    return crow::response(200, token);
                }
            }
            else {
                return crow::response(401, "Invalid username or password");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Verify MFA route
    CROW_ROUTE(app, "/verify-mfa")
        .methods("POST"_method)
        ([&dbManager](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x || !x.has("username") || !x.has("totp")) {
            return crow::response(400, "Invalid JSON or missing username/totp");
        }

        try {
            std::string totpSecret = dbManager.getTOTPSecret(x["username"].s());
            if (TOTP::verifyTOTP(totpSecret, x["totp"].s())) {
                std::string token = createToken(x["username"].s());
                return crow::response(200, token);
            }
            else {
                return crow::response(401, "Invalid TOTP code");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    });

    // Start the server
    app.port(18080).multithreaded().run();
}