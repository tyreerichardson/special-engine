#ifndef CASSCONNECT_HPP
#define CASSCONNECT_HPP

#include <cassandra.h>
#include <string>
#include <iostream>
#include <vector>
#include "message.hpp"



class CassConnect {
private: 
    CassCluster* cluster = cass_cluster_new();
    CassSession* session = cass_session_new();
    CassFuture* connect_future;
    CassFuture* prep_future;
    CassFuture* result_future;
    CassStatement* statement;

    const char* server_host = "127.0.0.1";

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

public:

    CassConnect() {
        cass_cluster_set_contact_points(cluster, this->server_host);
            this->connect_future = cass_session_connect(session, cluster);
        check_future(connect_future, "Unable to connect");
        cass_future_free(connect_future);
        std::cout << "Connected to CassDB" << std::endl;      
    }

    //loads chats
    std::vector<Message> load_messages(std::string username) {
        std::string query = 
            "SELECT message_id , message_from, message_to, content FROM special_engine.messages "
            "WHERE message_from = ? ALLOW FILTERING";

            const CassPrepared* prepared = nullptr;
            prep_future = cass_session_prepare(session, query.c_str());
            check_future(prep_future, "Query preparation failed");
            prepared = cass_future_get_prepared(prep_future);
            cass_future_free(prep_future);

            statement = cass_prepared_bind(prepared);
            cass_statement_bind_string(statement, 0, "user123");

            result_future = cass_session_execute(session, statement);
            check_future(result_future, "Query execution failed");

            const CassResult* result = cass_future_get_result(result_future);
            CassIterator* rows = cass_iterator_from_result(result);

            std::cout << "=== Messages where " << "message_from" << " = " << "user123" << " ===" << std::endl;
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
                std::cout << "From: " << std::string(from, from_len)
                          << "\nTo: " << std::string(to, to_len)
                          << "\nBody: " << std::string(body, body_len) << std::endl;

            }

            cass_iterator_free(rows);
            cass_result_free(result);
            cass_future_free(result_future);
            cass_statement_free(statement);
            cass_prepared_free(prepared);

            // Cleanup
            cass_cleanup();
            
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