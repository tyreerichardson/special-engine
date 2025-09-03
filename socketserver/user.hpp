#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <ctime>
#include <sstream>
#include <boost/json.hpp>

class User {
public:
    std::string user_id;
    std::string username;
    std::string created_at;
    std::string is_active;

    User() = default;

    User(const std::string& id, const std::string& user)
        : user_id(id), username(user) {
        std::time_t now = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        created_at = ss.str();
    }

    boost::json::object to_json() const {
        return {
            {"user_id", user_id},
            {"username", username},
            {"created_at", created_at},
            {"is_active", is_active}
        };
    }

    static User from_json(const std::string& json_str) {
        boost::json::value jv = boost::json::parse(json_str);
        boost::json::object obj = jv.as_object();

        User user;
        user.user_id = boost::json::value_to<std::string>(obj["user_id"]);
        user.username = boost::json::value_to<std::string>(obj["username"]);
        user.created_at = boost::json::value_to<std::string>(obj["created_at"]);
        user.is_active = boost::json::value_to<std::string>(obj["is_active"]);
        return user;
    }

};

#endif // USER_HPP