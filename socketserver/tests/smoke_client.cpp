#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

using Stream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;

void send(Stream& ws, const boost::json::object& message) {
    const auto text = boost::json::serialize(message);
    ws.write(boost::asio::buffer(text));
}

boost::json::object receive(Stream& ws) {
    boost::beast::flat_buffer buffer;
    ws.read(buffer);
    return boost::json::parse(boost::beast::buffers_to_string(buffer.data())).as_object();
}

void check(bool result, const char* description) {
    if (!result) throw std::runtime_error(description);
}

void login(Stream& ws, const char* name) {
    ws.next_layer().connect({boost::asio::ip::make_address("127.0.0.1"), 1234});
    ws.handshake("127.0.0.1:1234", "/");
    ws.text(true);
    send(ws, {{"username", name}});
    // The current protocol has no login acknowledgment. A self-message proves readiness.
    send(ws, {{"message_from", name}, {"message_to", name}, {"content", "ready"}});
    const auto reply = receive(ws);
    check(reply.at("content").as_string() == "ready", "login readiness failed");
}

int main(int argc, char**) {
    try {
        boost::asio::io_context io;
        Stream alice(io), bob(io);
        login(alice, "ctest_alice");
        if (argc > 1) {
            std::cout << "HOLD READY" << std::endl;
            boost::beast::flat_buffer buffer;
            boost::system::error_code error;
            alice.read(buffer, error);
            check(bool(error), "expected shutdown to disconnect the client");
            return 0;
        }
        login(bob, "ctest_bob");
        send(alice, {{"message_from", "ctest_alice"}, {"message_to", "ctest_bob"},
                     {"content", "hello from alice"}});
        const auto forward = receive(bob);
        check(forward.at("message_from").as_string() == "ctest_alice" &&
              forward.at("message_to").as_string() == "ctest_bob" &&
              forward.at("content").as_string() == "hello from alice", "forward delivery failed");
        send(bob, {{"message_from", "ctest_bob"}, {"message_to", "ctest_alice"},
                   {"content", "hello from bob"}});
        const auto reply = receive(alice);
        check(reply.at("message_from").as_string() == "ctest_bob" &&
              reply.at("content").as_string() == "hello from bob", "reply delivery failed");
        send(alice, {{"message_from", "ctest_alice"}, {"message_to", "ctest_offline"},
                     {"content", "offline check"}});
        check(receive(alice).at("error").as_string() == "User ctest_offline not connected",
              "offline-recipient response failed");
        alice.close(boost::beast::websocket::close_code::normal);
        bob.close(boost::beast::websocket::close_code::normal);
        std::cout << "PASS: login, bidirectional routing, offline response, client close\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
