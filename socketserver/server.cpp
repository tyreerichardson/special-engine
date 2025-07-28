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
#include <boost/json.hpp>

#include "message.hpp"

using namespace std;
using boost::asio::ip::tcp; // Use TCP socket classes from Boost Asio
std::map<std::string, std::shared_ptr<tcp::socket>> clients;
boost::mutex clients_mutex;


void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        std::string buffer;

        while (true) {
            boost::asio::streambuf read_buf;
            boost::system::error_code error;

            boost::asio::read_until(*socket, read_buf, '\n', error);
            if (error == boost::asio::error::eof) break;
            else if (error) throw boost::system::system_error(error);

            std::istream stream(&read_buf);
            std::getline(stream, buffer); // Get full message line

            boost::json::value jv = boost::json::parse(buffer);
            boost::json::object obj = jv.as_object();

            std::string from = boost::json::value_to<std::string>(obj["message_from"]);
            std::string to = boost::json::value_to<std::string>(obj["message_to"]);
            std::string message_id = boost::json::value_to<std::string>(obj["message_id"]);

            // Register sender if not already
            {
                boost::mutex::scoped_lock lock(clients_mutex);
                if (clients.find(from) == clients.end()) {
                    clients[from] = socket;
                }
            }

            std::string serialized_msg = boost::json::serialize(obj) + "\n";

            // Send to recipient
            {
                boost::mutex::scoped_lock lock(clients_mutex);
                if (clients.find(to) != clients.end()) {
                    boost::asio::write(*clients[to], boost::asio::buffer(serialized_msg));
                } else {
                    std::string error_msg = "{\"error\":\"User " + to + " not connected\"}\n";
                    boost::asio::write(*socket, boost::asio::buffer(error_msg));
                }
            }
        }
    } catch (std::exception& e) {
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