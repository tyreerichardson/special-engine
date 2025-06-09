#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <iostream>
#include <ctime>
#include <string>
#include <iterator>
#include <algorithm>

#include <boost/lambda/lambda.hpp>
#include <boost/asio.hpp>


using namespace std;
using boost::asio::ip::tcp;

void handle_client(tcp::socket socket) {
    try {
        char data[1024];
        while (true) {
            std::memset(data, 0, sizeof(data));
            boost::system::error_code error;

            // Read message from client
            size_t length = socket.read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof) break; // Connection closed
            else if (error) throw boost::system::system_error(error);

            std::cout << "Client: " << data << std::endl;

            // Send response
            std::string response;
            std::cout << "You: ";
            std::getline(std::cin, response);
            boost::asio::write(socket, boost::asio::buffer(response), error);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {


	try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint({}, 1234)); //tcp::v4()

        std::cout << "Server started. Waiting for client..." << std::endl;

        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        handle_client(std::move(socket));
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
	return 0;
}
