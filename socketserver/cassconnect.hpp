#ifndef CASSCONNECT_HPP
#define CASSCONNECT_HPP

#include <cassandra.h>
#include <string>
#include <iostream>
#include <vector>

#include "message.hpp"
#include "user.hpp"

class CassConnect {
private: 
    CassCluster* cluster = cass_cluster_new();
    CassSession* session = cass_session_new();
    CassFuture* connect_future;
    CassFuture* prep_future;
    CassFuture* result_future;
    CassStatement* statement;

    const char* server_host = "127.0.0.1";

    // Helper: check Cassandra future results
    void check_future(CassFuture* future, const std::string& errorMsg) {
        CassError rc = cass_future_error_code(future);
        if (rc != CASS_OK) {
            const char* msg;
            size_t msg_length;
            cass_future_error_message(future, &msg, &msg_length);
            std::cerr << errorMsg << ": " << std::string(msg, msg_length) << std::endl;
            exit(1);
        }
    }

    // Helper: run simple query (no params)
    void execute_query(CassSession* session, const std::string& query) {
        CassStatement* statement = cass_statement_new(query.c_str(), 0);
        CassFuture* future = cass_session_execute(session, statement);
        check_future(future, "Query failed: " + query);
        cass_future_free(future);
        cass_statement_free(statement);
    }

public:

    CassConnect() {
        cass_cluster_set_contact_points(cluster, this->server_host);
            this->connect_future = cass_session_connect(session, cluster);
        check_future(connect_future, "Unable to connect");
        cass_future_free(connect_future);
        std::cout << "Connected to CassDB" << std::endl;      
    }

    //loads chats
    std::vector<Message> load_messages(const char* username) {
        std::string query = 
            "SELECT message_id, message_from, message_to, content FROM special_engine.messages "
            "WHERE message_from = ? ALLOW FILTERING";

            const CassPrepared* prepared = nullptr;
            prep_future = cass_session_prepare(session, query.c_str());
            check_future(prep_future, "Query preparation failed");
            prepared = cass_future_get_prepared(prep_future);
            cass_future_free(prep_future);

            statement = cass_prepared_bind(prepared);
            cass_statement_bind_string(statement, 0, username);

            result_future = cass_session_execute(session, statement);
            check_future(result_future, "Query execution failed");

            const CassResult* result = cass_future_get_result(result_future);
            CassIterator* rows = cass_iterator_from_result(result);

            std::cout << "=== Messages where " << "message_from" << " = " << username << " ===" << std::endl;
            std::vector<Message> messages;
            while(cass_iterator_next(rows)) {
                const CassRow* row = cass_iterator_get_row(rows);

                const char* from; size_t from_len;
                const char* to; size_t to_len;
                const char* body; size_t body_len;

                cass_value_get_string(cass_row_get_column(row, 1), &from, &from_len);
                cass_value_get_string(cass_row_get_column(row, 2), &to, &to_len);
                cass_value_get_string(cass_row_get_column(row, 3), &body, &body_len);

                Message message;
                message.message_from = std::string(from, from_len);
                message.message_to = std::string(to, to_len);
                message.content = std::string(body, body_len);
                messages.push_back(message);
                // std::cout << "From: " << std::string(from, from_len)
                //           << "\nTo: " << std::string(to, to_len)
                //           << "\nBody: " << std::string(body, body_len) << std::endl;

            }

            cass_iterator_free(rows);
            cass_result_free(result);
            cass_future_free(result_future);
            cass_statement_free(statement);
            cass_prepared_free(prepared);

            // Cleanup
            //cass_cleanup();
            
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

                std::cout << "Message count for user '" << user << "': " << count << "\n";
            } else {
                std::cout << "No results found for user '" << user << "'.\n";
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
            "INSERT INTO special_engine.messages (message_id , message_from, message_to, content, created_at) "
            "VALUES (uuid(), '" + message.message_from + "', '" + message.message_to + "', '" + message.content + "', toTimeStamp(now()));";

        execute_query(session, insert_query);
    }

    void save_message(std::string from, std::string to, std::string content) {
        std::string insert_query =             
            "INSERT INTO special_engine.messages (message_id , message_from, message_to, content, created_at) "
            "VALUES (uuid(), '" + from + "', '" + to + "', '" + content + "', toTimeStamp(now()));";

        execute_query(session, insert_query);
    }
    
    //Cleans up Cassandra connections
    void cass_cleanup() {
        CassFuture* close_future = cass_session_close(session);
        cass_future_wait(close_future);
        cass_future_free(close_future);
        cass_cluster_free(cluster);
        cass_session_free(session);
    }

};

#endif //CASSCONNECT_HPP
