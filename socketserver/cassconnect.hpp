#ifndef CASSCONNECT_HPP
#define CASSCONNECT_HPP

#include <cassandra.h>
#include <string>
#include <iostream>

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

};

#endif //CASSCONNECT_HPP
