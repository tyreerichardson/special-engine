// server.hpp
//
// Manages active WebSocket client sessions.
// Provides thread-safe methods for registering, retrieving, and removing client connections
// based on their unique identifier (e.g., username or client ID).
//
// Author: Tyree Richardson
// Date: 2025-07-27

// Basic includes for standard library functionality
#include <iostream>     // For input/output streams
#include <ctime>        // (Unused in current code) For time utilities
#include <string>       // For string manipulation
#include <iterator>     // (Unused) Could be used for algorithms
#include <algorithm>    // (Unused) For utility algorithms

// Boost includes for threading and networking
#include <boost/thread.hpp>                         // Boost threads for handling clients
#include <boost/thread/mutex.hpp>                   // (Unused here) Could be used for shared resource protection
#include <boost/bind/bind.hpp>                      // For binding function arguments
#include <boost/lambda/lambda.hpp>                  // (Unused) For lambda expressions
#include <boost/asio.hpp>                           // Boost Asio networking
#include <boost/shared_ptr.hpp>                     // (Not used, replaced by std::shared_ptr)
#include <boost/enable_shared_from_this.hpp>        // (Unused) Useful when shared_ptrs reference self

using namespace std;
using boost::asio::ip::tcp; // Use TCP socket classes from Boost Asio

// Handles communication with a single client on its own thread
void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        char data[1024]; // Buffer to hold client message data

        while (true) {
            std::memset(data, 0, sizeof(data)); // Clear buffer for new data
            boost::system::error_code error;

            // Attempt to read incoming message from client
            size_t length = socket->read_some(boost::asio::buffer(data), error);

            // Check if client disconnected cleanly
            if (error == boost::asio::error::eof) break;

            // Throw if another error occurred
            else if (error) throw boost::system::system_error(error);

            // Print the received message to console
            std::cout << "Client: " << std::string(data, length) << std::endl;

            // Prepare and send response message back to client
            std::string response = "Server received: " + std::string(data, length);
            boost::asio::write(*socket, boost::asio::buffer(response), error);
        }

    } catch (std::exception& e) {
        // Log any exceptions thrown during communication
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    try {
        // Set up the I/O context used for managing asynchronous operations
        boost::asio::io_context io_context;

        // Create a TCP acceptor listening on port 1234 for IPv4
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1234)); 

        std::cout << "Server started. Waiting for client..." << std::endl;

        while(true){
            // Create a shared socket pointer for the incoming client connection
            std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(io_context);

            // Block and wait for a new client to connect
            acceptor.accept(*socket);
            std::cout << "New Client connected!" << std::endl;

            // Start a new thread for the connected client using Boost threads
            boost::thread t(handle_client, socket);

            // Detach the thread to allow it to run independently
            t.detach();

            // Note: You could also call handle_client synchronously here, but
            // that would block new connections until the current one finishes.
        }

    } catch (std::exception& e) {
        // Catch and log any errors related to the acceptor or server
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}