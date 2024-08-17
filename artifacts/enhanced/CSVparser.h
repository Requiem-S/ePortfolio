/*
 * File: CSVparser.h
 * Author: Thomas Gallegos
 * Email: N/A
 * Date: July 23, 2024
 * Version: 1.0
 *
 * Purpose:
 * This file defines the interface for the CSV parsing functionality.
 * It includes classes for error handling, row representation, and the main parser.
 *
 * Dependencies: None
 *
 */

#pragma once
#include <vector>
#include <string>
#include <iostream>

namespace csv {
    // Custom error class for CSV parsing errors
    class Error : public std::exception {
    public:
        Error(const std::string& msg) : _msg(msg) {}
        const char* what() const noexcept { return _msg.c_str(); }
    private:
        std::string _msg;
    };

    // Class representing a row in the CSV
    class Row {
    public:
        Row(const std::vector<std::string>& header);
        ~Row();
        unsigned int size() const;
        void push(const std::string&);
        bool set(const std::string&, const std::string&);
        const std::string operator[](unsigned int) const;
        const std::string operator[](const std::string& valuePosition) const;
        friend std::ostream& operator<<(std::ostream& os, const Row& row);
        friend std::ofstream& operator<<(std::ofstream& os, const Row& row);
    private:
        const std::vector<std::string> _header;
        std::vector<std::string> _values;
    };

    // Enum for specifying the data source type
    enum class DataType {
        eFILE = 0,
        ePURE = 1
    };

    // Main parser class
    class Parser {
    public:
        Parser(const std::string&, const DataType& type = DataType::eFILE, char sep = ',');
        ~Parser();
        Row& getRow(unsigned int row) const;
        unsigned int rowCount() const;
        unsigned int columnCount() const;
        std::vector<std::string> getHeader() const;
        const std::string getHeaderElement(unsigned int pos) const;
        const std::string& getFileName() const;
        bool deleteRow(unsigned int row);
        bool addRow(unsigned int pos, const std::vector<std::string>&);
        void sync() const;
        Row& operator[](unsigned int row) const;
    private:
        void parseHeader();
        void parseContent();
        std::string _file;
        const DataType _type;
        const char _sep;
        std::vector<std::string> _originalFile;
        std::vector<std::string> _header;
        std::vector<Row*> _content;
    };
}
