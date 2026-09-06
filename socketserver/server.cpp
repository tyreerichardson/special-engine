// Blocking WebSocket baseline. The async session/write-queue redesign is Phase 1.
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#ifdef ENABLE_CASSANDRA
#include "cassconnect.hpp"
#endif

using boost::asio::ip::tcp;
using Stream = boost::beast::websocket::stream<tcp::socket>;
namespace {
static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "signal handling requires lock-free bool atomics");
std::atomic<bool> stop_requested{false};
void request_stop(int) { stop_requested = 1; }
std::map<std::string, std::shared_ptr<Stream>> clients;
std::mutex clients_mutex;
#ifdef ENABLE_CASSANDRA
std::unique_ptr<CassConnect> cass_db;
#endif

void handle_client(const std::shared_ptr<Stream>& ws) {
    std::string username;
    try {
        ws->accept();
        boost::beast::flat_buffer login;
        ws->read(login);
        const auto value = boost::json::parse(boost::beast::buffers_to_string(login.data()));
        username = boost::json::value_to<std::string>(value.as_object().at("username"));
        if (username.empty()) throw std::runtime_error("username must not be empty");
#ifdef ENABLE_CASSANDRA
        if (!cass_db->user_exists(username)) cass_db->create_user(username);
#endif
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (clients.count(username)) throw std::runtime_error("username already connected");
            clients[username] = ws;
        }
        while (!stop_requested) {
            boost::beast::flat_buffer buffer;
            ws->read(buffer);
            const auto message = boost::beast::buffers_to_string(buffer.data());
            const auto value = boost::json::parse(message);
            const auto& obj = value.as_object();
            const auto from = boost::json::value_to<std::string>(obj.at("message_from"));
            const auto to = boost::json::value_to<std::string>(obj.at("message_to"));
            const auto content = boost::json::value_to<std::string>(obj.at("content"));
            if (from != username) continue;
#ifdef ENABLE_CASSANDRA
            cass_db->save_message(from, to, content);
#endif
            std::lock_guard<std::mutex> lock(clients_mutex);
            const auto recipient = clients.find(to);
            if (recipient != clients.end()) {
                recipient->second->text(true);
                recipient->second->write(boost::asio::buffer(message));
            } else {
                const auto error = boost::json::serialize(boost::json::object{
                    {"error", "User " + to + " not connected"}});
                ws->text(true);
                ws->write(boost::asio::buffer(error));
            }
        }
    } catch (const boost::system::system_error& error) {
        if (!stop_requested && error.code() != boost::beast::websocket::error::closed)
            std::cerr << "Client error: " << error.what() << '\n';
    } catch (const std::exception& error) {
        if (!stop_requested) std::cerr << "Client error: " << error.what() << '\n';
    }
    std::lock_guard<std::mutex> lock(clients_mutex);
    const auto found = clients.find(username);
    if (found != clients.end() && found->second == ws) clients.erase(found);
}

struct Worker {
    std::shared_ptr<Stream> stream;
    // A duplicate descriptor keeps the shutdown handle valid even if Beast closes
    // the original socket during a read/handshake failure. It never owns a new connection.
    int shutdown_fd;
    std::atomic<bool> finished{false};
    std::thread thread;
    explicit Worker(std::shared_ptr<Stream> s) : stream(std::move(s)),
        shutdown_fd(::dup(stream->next_layer().native_handle())) {
        if (shutdown_fd < 0) throw std::runtime_error("could not duplicate socket for shutdown");
    }
    ~Worker() { ::close(shutdown_fd); }
};
}

int main() {
    std::signal(SIGINT, request_stop);
    std::signal(SIGTERM, request_stop);
    std::signal(SIGPIPE, SIG_IGN);
    boost::asio::io_context io;
    std::vector<std::unique_ptr<Worker>> workers;
    int result = 0;
    try {
#ifdef ENABLE_CASSANDRA
        cass_db.reset(new CassConnect);
        cass_db->load_messages("user123");
#else
        std::cout << "Persistence disabled: users and messages are not saved." << std::endl;
#endif
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 1234));
        acceptor.non_blocking(true);
        std::cout << "Server started. Waiting for client..." << std::endl;
        while (!stop_requested) {
            for (auto it = workers.begin(); it != workers.end();) {
                if ((*it)->finished.load()) {
                    (*it)->thread.join();
                    it = workers.erase(it);
                } else ++it;
            }
            tcp::socket socket(io);
            boost::system::error_code error;
            acceptor.accept(socket, error);
            if (error == boost::asio::error::would_block || error == boost::asio::error::try_again) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }
            if (error) throw boost::system::system_error(error);
            auto stream = std::make_shared<Stream>(std::move(socket));
            workers.emplace_back(new Worker(stream));
            Worker* worker = workers.back().get();
            worker->thread = std::thread([worker] {
                handle_client(worker->stream);
                worker->finished.store(true);
            });
        }
        // Acceptor destruction stops new connections before worker teardown.
    } catch (const std::exception& error) {
        std::cerr << "Server error: " << error.what() << std::endl;
        result = 1;
    }
    stop_requested = 1;
    // POSIX shutdown interrupts blocking I/O without concurrently invoking Beast
    // methods. Mac and Ubuntu support it. Streams remain alive until threads join.
    for (auto& worker : workers) ::shutdown(worker->shutdown_fd, SHUT_RDWR);
    for (auto& worker : workers) if (worker->thread.joinable()) worker->thread.join();
    workers.clear();
    clients.clear();
#ifdef ENABLE_CASSANDRA
    cass_db.reset();
#endif
    if (result == 0) std::cout << "Server stopped cleanly." << std::endl;
    return result;
}
