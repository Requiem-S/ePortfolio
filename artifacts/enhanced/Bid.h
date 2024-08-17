/*
 * File: Bid.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file defines the Bid structure, which represents a single bid in the
 * Bid Management System. It contains all the relevant information for a bid.
 *
 * Dependencies: None
 *
 */

#pragma once
#include <string>

struct Bid {
    std::string auctionTitle;
    std::string auctionId;
    std::string department;
    std::string closeDate;
    double winningBid;
    double ccFee;
    double feePercent;
    double auctionFeeSubtotal;
    double auctionFeeTotal;
    std::string payStatus;
    std::string paidDate;
    std::string assetNumber;
    std::string inventoryId;
    std::string decalVehicleId;
    std::string vtrNumber;
    std::string receiptNumber;
    double cap;
    double expenses;
    double netSales;
    std::string fund;
    std::string businessUnit;

    // Default constructor initializing numeric fields to 0
    Bid() : winningBid(0), ccFee(0), feePercent(0), auctionFeeSubtotal(0), auctionFeeTotal(0), cap(0), expenses(0), netSales(0) {}
};
