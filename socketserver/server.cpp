#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>

#include <iostream>
#include <ctime>
#include <string>
#include <iterator>
#include <algorithm>

#include <boost/lambda/lambda.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;
using boost::asio::ip::tcp;

void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        char data[1024];
        while (true) {
            std::memset(data, 0, sizeof(data));
            boost::system::error_code error;

            // Read message from client
            size_t length = socket->read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof) break; // client disconnected
            else if (error) throw boost::system::system_error(error);

            std::cout << "Client: " << std::string(data, length) << std::endl;

            // Send response
            std::string response = "Server received: " + std::string(data, length);
            //std::cout << "You: ";
            //std::getline(std::cin, response);
            boost::asio::write(*socket, boost::asio::buffer(response), error);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
	try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1234)); 

        std::cout << "Server started. Waiting for client..." << std::endl;

        while(true){
            std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(io_context);

            //tcp::socket socket(io_context);
            acceptor.accept(*socket);
            std::cout << "New Client connected!" << std::endl;
            boost::thread t(handle_client, socket);
            t.detach();

            //handle_client(std::move(socket));
        }

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
	return 0;
}
