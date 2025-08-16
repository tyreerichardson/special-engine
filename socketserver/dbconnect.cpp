#include <cassandra.h>
#include <iostream>
#include <string>

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

int main(int argc, char* argv[]) {
    // Default to "message_from" + "user123" if no args
    std::string field = (argc > 1) ? argv[1] : "message_from";
    std::string value = (argc > 2) ? argv[2] : "user123";

    // Setup cluster and session
    CassCluster* cluster = cass_cluster_new();
    CassSession* session = cass_session_new();

    cass_cluster_set_contact_points(cluster, "127.0.0.1"); // update with your cluster

    CassFuture* connect_future = cass_session_connect(session, cluster);
    check_future(connect_future, "Unable to connect");
    cass_future_free(connect_future);

    // --- CREATE KEYSPACE & TABLE ---
    execute_query(session,
        "CREATE KEYSPACE IF NOT EXISTS special_engine "
        "WITH replication = {'class': 'SimpleStrategy', 'replication_factor': '1'};"
    );

    execute_query(session,
        "CREATE TABLE IF NOT EXISTS special_engine.messages ("
        "message_id UUID PRIMARY KEY, "
        "message_from text, "
        "message_to text, "
        "content text);"
    );

    // --- INSERT SAMPLE VALUES ---
    execute_query(session,
        "INSERT INTO special_engine.messages (message_id , message_from, message_to, content) "
        "VALUES (uuid(), 'user123', 'user456', 'Hello from user123!');"
    );
    execute_query(session,
        "INSERT INTO special_engine.messages (message_id , message_from, message_to, content) "
        "VALUES (uuid(), 'user456', 'user123', 'Hi user123, got your message!');"
    );

    // --- QUERY ---
    std::string query =
        "SELECT message_id , message_from, message_to, content FROM special_engine.messages "
        "WHERE " + field + " = ? ALLOW FILTERING";

    const CassPrepared* prepared = nullptr;
    CassFuture* prep_future = cass_session_prepare(session, query.c_str());
    check_future(prep_future, "Query preparation failed");
    prepared = cass_future_get_prepared(prep_future);
    cass_future_free(prep_future);

    // Bind param
    CassStatement* statement = cass_prepared_bind(prepared);
    cass_statement_bind_string(statement, 0, value.c_str());

    // Execute
    CassFuture* result_future = cass_session_execute(session, statement);
    check_future(result_future, "Query execution failed");

    // Process results
    const CassResult* result = cass_future_get_result(result_future);
    CassIterator* rows = cass_iterator_from_result(result);

    std::cout << "=== Messages where " << field << " = " << value << " ===" << std::endl;
    while (cass_iterator_next(rows)) {
        const CassRow* row = cass_iterator_get_row(rows);


        const char* from; size_t from_len;
        const char* to; size_t to_len;
        const char* body; size_t body_len;

        cass_value_get_string(cass_row_get_column(row, 1), &from, &from_len);
        cass_value_get_string(cass_row_get_column(row, 2), &to, &to_len);
        cass_value_get_string(cass_row_get_column(row, 3), &body, &body_len);

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
    CassFuture* close_future = cass_session_close(session);
    cass_future_wait(close_future);
    cass_future_free(close_future);
    cass_cluster_free(cluster);
    cass_session_free(session);

    return 0;
}
