/*
 * File: LinkedList.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file defines the LinkedList class, which provides an in-memory storage
 * solution for Bid objects. It includes declarations for various operations
 * like insertion, deletion, searching, and sorting.
 *
 * Dependencies:
 * - Bid.h for the Bid structure
 *
 */

#pragma once
#include "Bid.h"
#include <vector>

class LinkedList {
private:
    // Node structure for the linked list
    struct Node {
        Bid bid;
        Node* next;
        Node* prev;

        Node(Bid aBid) : bid(aBid), next(nullptr), prev(nullptr) {}
    };

    Node* head;
    Node* tail;
    int size;

    // Helper methods for Sort
    Node* mergeSort(Node* node, bool (*comparator)(const Bid&, const Bid&));
    Node* merge(Node* left, Node* right, bool (*comparator)(const Bid&, const Bid&));
    Node* getMiddle(Node* head);

public:
    LinkedList();
    ~LinkedList();
    void Append(const Bid& bid);
    void Prepend(const Bid& bid);
    void InsertAfter(const std::string& auctionId, const Bid& newBid);
    void Remove(const std::string& auctionId);
    Bid Search(const std::string& auctionId);
    std::vector<Bid> GetAllBids();
    int Size() const;
    void Reverse();

    // Sorting and searching methods
    void Sort(bool (*comparator)(const Bid&, const Bid&));
    Bid BinarySearch(const std::string& auctionId);
};
