#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#pragma once
#include <string>
#include <ctime>
#include <sstream>
#include <boost/json.hpp>

class Message {
public:
    std::string message_id;
    std::string message_from;
    std::string message_to;
    std::string content;
    std::string created_at;

    Message() = default;

    Message(const std::string& id, const std::string& from, const std::string& to, const std::string& msg)
        : message_id(id), message_from(from), message_to(to), content(msg) {
        std::time_t now = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        created_at = ss.str();
    }

    boost::json::object to_json() const {
        return {
            {"message_id", message_id},
            {"message_from", message_from},
            {"message_to", message_to},
            {"content", content},
            {"created_at", created_at}
        };
    }

    static Message from_json(const std::string& json_str) {
        boost::json::value jv = boost::json::parse(json_str);
        boost::json::object obj = jv.as_object();

        Message msg;
        msg.message_id = boost::json::value_to<std::string>(obj["message_id"]);
        msg.message_from = boost::json::value_to<std::string>(obj["message_from"]);
        msg.message_to = boost::json::value_to<std::string>(obj["message_to"]);
        msg.content = boost::json::value_to<std::string>(obj["content"]);
        msg.created_at = boost::json::value_to<std::string>(obj["created_at"]);
        return msg;
    }
};

#endif // MESSAGE_HPP
