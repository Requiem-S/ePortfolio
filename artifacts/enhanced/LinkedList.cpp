/*
 * File: LinkedList.cpp
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file implements the LinkedList class, which provides an in-memory storage
 * solution for Bid objects. It includes various operations like insertion, deletion,
 * searching, and sorting.
 *
 * Dependencies:
 * - Bid.h for the Bid structure
 *
 */

#include "LinkedList.h"
#include <algorithm>

LinkedList::LinkedList() : head(nullptr), tail(nullptr), size(0) {}

LinkedList::~LinkedList() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

// Append a new bid to the end of the list
void LinkedList::Append(const Bid& bid) {
    Node* newNode = new Node(bid);
    if (head == nullptr) {
        head = tail = newNode;
    }
    else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    size++;
}

// Prepend a new bid to the beginning of the list
void LinkedList::Prepend(const Bid& bid) {
    Node* newNode = new Node(bid);
    if (head == nullptr) {
        head = tail = newNode;
    }
    else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    size++;
}

// Insert a new bid after a specified auction ID
void LinkedList::InsertAfter(const std::string& auctionId, const Bid& newBid) {
    Node* current = head;
    while (current != nullptr) {
        if (current->bid.auctionId == auctionId) {
            Node* newNode = new Node(newBid);
            newNode->next = current->next;
            newNode->prev = current;
            if (current->next) current->next->prev = newNode;
            current->next = newNode;
            if (current == tail) tail = newNode;
            size++;
            return;
        }
        current = current->next;
    }
}

// Remove a bid with the specified auction ID
void LinkedList::Remove(const std::string& auctionId) {
    Node* current = head;
    while (current != nullptr) {
        if (current->bid.auctionId == auctionId) {
            if (current->prev) current->prev->next = current->next;
            if (current->next) current->next->prev = current->prev;
            if (current == head) head = current->next;
            if (current == tail) tail = current->prev;
            delete current;
            size--;
            return;
        }
        current = current->next;
    }
}

// Search for a bid by auction ID
Bid LinkedList::Search(const std::string& auctionId) {
    Node* current = head;
    while (current != nullptr) {
        if (current->bid.auctionId == auctionId) {
            return current->bid;
        }
        current = current->next;
    }
    return Bid(); // Return empty bid if not found
}

// Get all bids in the list
std::vector<Bid> LinkedList::GetAllBids() {
    std::vector<Bid> bids;
    Node* current = head;
    while (current != nullptr) {
        bids.push_back(current->bid);
        current = current->next;
    }
    return bids;
}
// Get the size of the list
int LinkedList::Size() const {
    return size;
}

// Reverse the order of the list
void LinkedList::Reverse() {
    Node* current = head;
    Node* temp = nullptr;

    while (current != nullptr) {
        temp = current->prev;
        current->prev = current->next;
        current->next = temp;
        current = current->prev;
    }

    if (temp != nullptr) {
        head = temp->prev;
    }

    std::swap(head, tail);
}

// Sort the list using merge sort
void LinkedList::Sort(bool (*comparator)(const Bid&, const Bid&)) {
    if (head == nullptr || head->next == nullptr) {
        return; // List is empty or has only one element
    }

    head = mergeSort(head, comparator);

    // Update tail and prev pointers after sorting
    Node* current = head;
    current->prev = nullptr;
    while (current->next != nullptr) {
        current->next->prev = current;
        current = current->next;
    }
    tail = current;
}

// Helper function for merge sort
LinkedList::Node* LinkedList::mergeSort(Node* node, bool (*comparator)(const Bid&, const Bid&)) {
    if (node == nullptr || node->next == nullptr) {
        return node;
    }

    Node* middle = getMiddle(node);
    Node* nextToMiddle = middle->next;

    middle->next = nullptr;

    Node* left = mergeSort(node, comparator);
    Node* right = mergeSort(nextToMiddle, comparator);

    return merge(left, right, comparator);
}

// Helper function to merge two sorted lists
LinkedList::Node* LinkedList::merge(Node* left, Node* right, bool (*comparator)(const Bid&, const Bid&)) {
    if (left == nullptr) {
        return right;
    }
    if (right == nullptr) {
        return left;
    }

    Node* result;
    if (comparator(left->bid, right->bid)) {
        result = left;
        result->next = merge(left->next, right, comparator);
    }
    else {
        result = right;
        result->next = merge(left, right->next, comparator);
    }

    return result;
}

// Helper function to get the middle node of the list
LinkedList::Node* LinkedList::getMiddle(Node* head) {
    if (head == nullptr) {
        return head;
    }

    Node* slow = head;
    Node* fast = head->next;

    while (fast != nullptr) {
        fast = fast->next;
        if (fast != nullptr) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    return slow;
}

// Perform binary search on the sorted list
Bid LinkedList::BinarySearch(const std::string& auctionId) {
    // First, sort the list by auctionId
    Sort([](const Bid& a, const Bid& b) { return a.auctionId < b.auctionId; });

    Node* start = head;
    Node* end = tail;

    while (start && end && start != end->next) {
        Node* mid = getMiddle(start);

        if (mid->bid.auctionId == auctionId) {
            return mid->bid;
        }

        if (mid->bid.auctionId < auctionId) {
            start = mid->next;
        }
        else {
            end = mid->prev;
        }
    }

    return Bid(); // Return empty bid if not found
}