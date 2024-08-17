cmake_minimum_required(VERSION 3.10)
project(BidManagementSystem)

# Set OpenSSL root directory and preferences
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_MSVC_STATIC_RT TRUE)

# Explicitly set OpenSSL paths
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/VC/x64/MT/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/VC/x64/MT/libssl.lib")

# Find OpenSSL package
find_package(OpenSSL REQUIRED)


include_directories(${OPENSSL_INCLUDE_DIR})

# Print OpenSSL variables for debugging
message(STATUS "OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")
message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SQLiteCpp
add_subdirectory(C:/Users/Adult/source/repos/BidManagementServer/SQLiteCpp-master)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SQLiteCpp-master/include)

# Your source files
set(SOURCE_FILES
    BidManagementServer.cpp
    DatabaseManager.cpp
    Bid.cpp
    LinkedList.cpp
    CSVparser.cpp
    # Add any other .cpp files your project uses
)

set(HEADER_FILES
    Bid.h
    DatabaseManager.h
    LinkedList.h
    TOTP.h
    User.h
    Utils.h
    CSVparser.h
)

# Your executable
add_executable(BidManagementSystem ${SOURCE_FILES} ${HEADER_FILES})

# Link libraries
target_link_libraries(BidManagementSystem
    SQLiteCpp
    crow
    sqlite3
    jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
       
    # Add any other libraries your project uses
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SQLiteCpp-master/include)
