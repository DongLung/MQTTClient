#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

#include <mqtt/async_client.h>

struct Config {
    std::string server;
    int port = 1883;
    std::vector<std::string> topics;
    std::vector<std::string> subs;
    std::string statusTopic;
};

// Trim whitespace from a string
static std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto start = s.find_first_not_of(ws);
    if (start == std::string::npos)
        return "";
    auto end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// Parse simple key=value configuration with comma separated lists
static Config parseConfig(const std::string& path) {
    Config cfg;
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open config file: " + path);
    }
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        auto pos = line.find('=');
        if (pos == std::string::npos)
            continue;
        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        if (key == "MQTT_SERVER") {
            cfg.server = value;
        } else if (key == "MQTT_PORT") {
            cfg.port = std::stoi(value);
        } else if (key == "TOPIC_LIST") {
            std::stringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                item = trim(item);
                if (!item.empty())
                    cfg.topics.push_back(item);
            }
        } else if (key == "SUBSCRIBE_LIST") {
            std::stringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                item = trim(item);
                if (!item.empty())
                    cfg.subs.push_back(item);
            }
        } else if (key == "STATUS_TOPIC") {
            cfg.statusTopic = value;
        }
    }
    return cfg;
}

static std::string nowString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    #ifdef _WIN32
        localtime_s(&tm, &now_c);
    #else
        localtime_r(&now_c, &tm);
    #endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d%H%M%S");
    return oss.str();
}

class Callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener {
    mqtt::async_client& cli_;
    mqtt::connect_options& connOpts_;
    bool firstConnect_{true};
    Config& cfg_;
public:
    Callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, Config& cfg)
        : cli_(cli), connOpts_(connOpts), cfg_(cfg) {}

    void on_failure(const mqtt::token& tok) override {
        std::cerr << "Connection attempt failed" << std::endl;
    }

    void on_success(const mqtt::token& tok) override {}

    void connected(const std::string& cause) override {
        std::cout << "Connected" << std::endl;
        for (const auto& t : cfg_.subs) {
            cli_.subscribe(t, 1);
        }
        if (firstConnect_) {
            firstConnect_ = false;
            std::string payload = std::string("{ \"conTime\": \"") + nowString() + "\" }";
            mqtt::message msg(cfg_.statusTopic, payload, 1, true);
            cli_.publish(msg);
        }
    }

    void connection_lost(const std::string& cause) override {
        std::cerr << "Connection lost: " << cause << std::endl;
        while (true) {
            try {
                cli_.reconnect();
                break;
            } catch (const mqtt::exception& exc) {
                std::cerr << "Reconnect failed: " << exc.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        std::cout << "Message arrived on topic: " << msg->get_topic() << " payload: " << msg->to_string() << std::endl;
    }

    void delivery_complete(mqtt::delivery_token_ptr tok) override {}
};

int main(int argc, char* argv[]) {
    try {
        Config cfg = parseConfig("mqttClient.ini");
        std::string address = "tcp://" + cfg.server + ":" + std::to_string(cfg.port);
        mqtt::async_client client(address, "mqttClient");
        mqtt::connect_options connOpts;
        connOpts.set_clean_session(true);

        Callback cb(client, connOpts, cfg);
        client.set_callback(cb);

        client.connect(connOpts, nullptr, cb);

        // Simple loop to publish to topics (for demonstration)
        int count = 0;
        while (true) {
            for (const auto& t : cfg.topics) {
                std::ostringstream oss;
                oss << "Message " << count++;
                mqtt::message msg(t, oss.str(), 1, false);
                client.publish(msg);
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        client.disconnect()->wait();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

