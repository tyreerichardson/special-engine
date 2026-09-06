#ifndef CASSCONNECT_HPP
#define CASSCONNECT_HPP

#include <cassandra.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>

#include "message.hpp"
#include "user.hpp"

class CassConnect {
private: 
    CassCluster* cluster = cass_cluster_new();
    CassSession* session = cass_session_new();
    using Future = std::unique_ptr<CassFuture, decltype(&cass_future_free)>;
    using Statement = std::unique_ptr<CassStatement, decltype(&cass_statement_free)>;
    using Prepared = std::unique_ptr<const CassPrepared, decltype(&cass_prepared_free)>;
    using Result = std::unique_ptr<const CassResult, decltype(&cass_result_free)>;
    using Iterator = std::unique_ptr<CassIterator, decltype(&cass_iterator_free)>;

    const char* server_host = "127.0.0.1";

    // Helper: check Cassandra future results
    void check_future(CassFuture* future, const std::string& errorMsg) {
        CassError rc = cass_future_error_code(future);
        if (rc != CASS_OK) {
            const char* msg;
            size_t msg_length;
            cass_future_error_message(future, &msg, &msg_length);
            std::cerr << errorMsg << ": " << std::string(msg, msg_length) << std::endl;
            throw std::runtime_error(errorMsg + ": " + std::string(msg, msg_length));
        }
    }

    // Helper: run simple query (no params)
    void execute_query(CassSession* session, const std::string& query) {
        CassStatement* statement = cass_statement_new(query.c_str(), 0);
        CassFuture* future = cass_session_execute(session, statement);
        try {
            check_future(future, "Query failed: " + query);
        } catch (...) {
            cass_future_free(future);
            cass_statement_free(statement);
            throw;
        }
        cass_future_free(future);
        cass_statement_free(statement);
    }

public:

    CassConnect() {
        cass_cluster_set_contact_points(cluster, this->server_host);
        cass_cluster_set_connect_timeout(cluster, 3000);
        cass_cluster_set_request_timeout(cluster, 3000);
        CassFuture* future = cass_session_connect(session, cluster);
        try {
            check_future(future, "Unable to connect");
        } catch (...) {
            cass_future_free(future);
            cass_session_free(session);
            cass_cluster_free(cluster);
            throw;
        }
        cass_future_free(future);
        std::cout << "Connected to CassDB" << std::endl;
    }

    CassConnect(const CassConnect&) = delete;
    CassConnect& operator=(const CassConnect&) = delete;
    ~CassConnect() { cass_cleanup(); }

    //loads chats
    std::vector<Message> load_messages(const char* username) {
        const char* query = "SELECT message_id, message_from, message_to, content "
            "FROM special_engine.messages WHERE message_from = ? "
            "AND created_day > current_date() - 30d ALLOW FILTERING";
        Future preparation(cass_session_prepare(session, query), cass_future_free);
        check_future(preparation.get(), "Query preparation failed");
        Prepared prepared(cass_future_get_prepared(preparation.get()), cass_prepared_free);
        Statement statement(cass_prepared_bind(prepared.get()), cass_statement_free);
        cass_statement_bind_string(statement.get(), 0, username);
        Future future(cass_session_execute(session, statement.get()), cass_future_free);
        check_future(future.get(), "Query execution failed");
        Result result(cass_future_get_result(future.get()), cass_result_free);
        Iterator rows(cass_iterator_from_result(result.get()), cass_iterator_free);
        std::vector<Message> messages;
        while (cass_iterator_next(rows.get())) {
            const CassRow* row = cass_iterator_get_row(rows.get());
            const char *from, *to, *body;
            size_t from_len, to_len, body_len;
            if (cass_value_get_string(cass_row_get_column(row, 1), &from, &from_len) != CASS_OK ||
                cass_value_get_string(cass_row_get_column(row, 2), &to, &to_len) != CASS_OK ||
                cass_value_get_string(cass_row_get_column(row, 3), &body, &body_len) != CASS_OK)
                throw std::runtime_error("Invalid message row");
            Message message;
            message.message_from.assign(from, from_len);
            message.message_to.assign(to, to_len);
            message.content.assign(body, body_len);
            messages.push_back(message);
        }
        return messages;
    }

    //gets messages
    Message get_messages(std::string username) {
        std::string query =
            "SELECT message_id , message_from, message_to, content FROM special_engine.messages "
            "WHERE message_from = ? ALLOW FILTERING";
        
        Message message;
        return message;
    }

    void create_user(std::string username) {
        std::string insert_query = 
        "INSERT INTO special_engine.users(user_id, username, created_at, is_active) "
        "VALUES (uuid(), '" + username + "', toTimeStamp(now()), false)";
        execute_query(session, insert_query);
    }

    bool user_exists(const std::string& user) {
        bool found = false;
        const char* query =
            "SELECT COUNT(1) FROM special_engine.users WHERE username = ? ALLOW FILTERING;";

        CassStatement* statement = cass_statement_new(query, 1);
        cass_statement_bind_string(statement, 0, user.c_str());

        CassFuture* result_future = cass_session_execute(session, statement);
        cass_statement_free(statement);

        if (cass_future_error_code(result_future) == CASS_OK) {
            const CassResult* result = cass_future_get_result(result_future);
            CassIterator* rows = cass_iterator_from_result(result);

            if (cass_iterator_next(rows)) {
                const CassRow* row = cass_iterator_get_row(rows);
                cass_int64_t count = 0;

                const CassValue* value = cass_row_get_column(row, 0);
                cass_value_get_int64(value, &count);
                found = (count>0) ? true : false;
            }
            
            cass_iterator_free(rows);
            cass_result_free(result);
        } else {
            const char* message;
            size_t message_length;
            cass_future_error_message(result_future, &message, &message_length);
            std::cerr << "Query failed: " << std::string(message, message_length) << "\n";
        }

        cass_future_free(result_future);
        return found;
    }

    void save_message(Message message) {
        std::string insert_query =             
            "INSERT INTO special_engine.messages (message_id , message_from, message_to, content, created_day, created_at) "
            "VALUES (uuid(), '" + message.message_from + "', '" + message.message_to + "', '" + message.content + "', current_date(), toTimeStamp(now()));";

        execute_query(session, insert_query);
    }

    void save_message(std::string from, std::string to, std::string content) {
        std::string insert_query =             
            "INSERT INTO special_engine.messages (message_id , message_from, message_to, content, created_day, created_at) "
            "VALUES (uuid(), '" + from + "', '" + to + "', '" + content + "', current_date(), toTimeStamp(now()));";

        execute_query(session, insert_query);
    }
    
    //Cleans up Cassandra connections
    void cass_cleanup() {
        if (!session) return;
        CassFuture* close_future = cass_session_close(session);
        cass_future_wait(close_future);
        cass_future_free(close_future);
        cass_cluster_free(cluster);
        cass_session_free(session);
        session = nullptr;
        cluster = nullptr;
    }

};

#endif //CASSCONNECT_HPP
