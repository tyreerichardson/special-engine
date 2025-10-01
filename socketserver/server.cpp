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

// Boost includes for threading and 
#include <boost/beast/core.hpp>                     // Buffers for store incoming/outgoing data safely
#include <boost/beast/websocket.hpp>                // Raw TCP connection insto a full WebSokcet stream(HandShake Upgraded)
#include <boost/thread.hpp>                         // Boost threads for handling clients
#include <boost/thread/mutex.hpp>                   // (Unused here) Could be used for shared resource protection
#include <boost/bind/bind.hpp>                      // For binding function arguments
#include <boost/lambda/lambda.hpp>                  // (Unused) For lambda expressions
#include <boost/asio.hpp>                           // Boost Asio networking
#include <boost/shared_ptr.hpp>                     // (Not used, replaced by std::shared_ptr)
#include <boost/enable_shared_from_this.hpp>        // (Unused) Useful when shared_ptrs reference self
#include <boost/json.hpp>

#include "message.hpp"
#include "cassconnect.hpp"

using namespace std;
using boost::asio::ip::tcp; // Use TCP socket classes from Boost Asio
std::map<std::string, std::shared_ptr<boost::beast::websocket::stream<tcp::socket>>> clients;
boost::mutex clients_mutex;
CassConnect cass_db;

std::string log_in(std::shared_ptr<boost::beast::websocket::stream<tcp::socket>>& ws) {
    std::string username = "";
    try {
        boost::beast::flat_buffer buffer;
        ws->read(buffer);

        std::string msg = boost::beast::buffers_to_string(buffer.data());
        std::cout << "Received: " << msg << std::endl;

        boost::json::value jv = boost::json::parse(msg);
        boost::json::object obj = jv.as_object();

        username = boost::json::value_to<std::string>(obj["username"]);

        //TODO: this will change once logging is added
        if(!cass_db.user_exists(username.c_str())){
            std::cout << "Adding User to the table" << std::endl;
            cass_db.create_user(username.c_str());
        } else 
            std::cout << "Thank you for coming back " << username.c_str() << std::endl;

        // Register sender if not already
        {
            boost::mutex::scoped_lock lock(clients_mutex);
            if (clients.find(username) == clients.end()) {
                clients[username] = ws;
            }
        }
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return username;
}

void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        auto ws = std::make_shared<boost::beast::websocket::stream<tcp::socket>>(std::move(*socket));

        // Perform WebSocket handshake
        ws->accept();

        std::string username = log_in(ws);

        for (;;) {
            boost::beast::flat_buffer buffer;
            ws->read(buffer);

            std::string msg = boost::beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());
            
            std::cout << "Received: " << msg << std::endl;

            // Parse JSON message
            boost::json::value jv = boost::json::parse(msg);
            boost::json::object obj = jv.as_object();

            std::string from    = boost::json::value_to<std::string>(obj["message_from"]);
            std::string to      = boost::json::value_to<std::string>(obj["message_to"]);
            std::string content = boost::json::value_to<std::string>(obj["content"]);

            if(username!=from){
                std::cout << username << " tried sending a message with the username: " << from  << std::endl;
            } else { //SENDS MESSAGE WITH FAKE_AUTH

                // Save to Cassandra
                cass_db.save_message(from, to, content);

                // Register sender if not already
                {
                    boost::mutex::scoped_lock lock(clients_mutex);
                    if(clients.find(from) == clients.end()){
                        clients[from] = ws;
                    }
                }

                std::string serialized_msg = boost::json::serialize(obj) + "\n";

                // Send to recipient
                {
                    boost::mutex::scoped_lock lock(clients_mutex);
                    if (clients.find(to) != clients.end()) {
                        clients[to]->text(true);
                        clients[to]->write(boost::asio::buffer(msg));
                    } else {
                        std::string error_msg = "{\"error\":\"User " + to + " not connected\"}\n";
                        ws->text(true);
                        ws->write(boost::asio::buffer(error_msg));
                    }
                }
            }           
        }

    } catch (std::exception& e) {
        std::cerr << "Exception in client: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::vector<Message> messages = cass_db.load_messages("user123");
        for(auto message : messages){
            //the message object returned from CassConnect
            std::cout << "From: " << message.message_from << std::endl;
	        std::cout << "  To: " << message.message_to << std::endl;
	        std::cout << " msg: " << message.content << std::endl;
        }

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