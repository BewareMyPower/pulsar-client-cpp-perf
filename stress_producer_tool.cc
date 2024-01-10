#include <pulsar/Client.h>

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "LogUtils.h"
#include "ini_parser.h"
#include "rate_limiter.h"

DECLARE_LOG_OBJECT()

using namespace pulsar;

int main(int argc, char *argv[]) {
    const auto config_path = argc > 1 ? argv[1] : "/etc/pulsar/config.ini";
    std::cout << "Loading configs from " << config_path << std::endl;
    const auto configs = parseIni(config_path);
    const int num_topics = configs.get<int>("num_topics");
    const int connections_per_broker = configs.get<int>("connections_per_broker");
    const auto log_path = configs.get_or_else<std::string>("log_path", "");
    const auto tls_trust_certs_file_path = configs.get_or_else<std::string>("tls_trust_certs_file_path", "");
    const int rate = configs.get<int>("send_rate");
    const int num_messages = configs.get<int>("num_messages");
    const int message_size = configs.get<int>("message_size");
    if (num_messages <= 0 || message_size <= 0) {
        std::cerr << "Wrong config of num_messages: " << num_messages << ", message_size: " << message_size
                  << std::endl;
        return 1;
    }

    ClientConfiguration conf;
    conf.setIOThreads(std::thread::hardware_concurrency());
    conf.setConnectionsPerBroker(connections_per_broker);
    if (!log_path.empty()) {
        conf.setLogger(new FileLoggerFactory(Logger::LEVEL_INFO, log_path));
    }
    if (!tls_trust_certs_file_path.empty()) {
        conf.setTlsTrustCertsFilePath(tls_trust_certs_file_path);
    }

    try {
        ParamMap params;
        params["issuer_url"] = configs["issuer_url"];
        params["client_id"] = configs["client_id"];
        params["client_secret"] = configs["client_secret"];
        params["audience"] = configs["audience"];
        conf.setAuth(pulsar::AuthOauth2::create(params));
    } catch (const std::runtime_error &e) {
        std::cout << "Failed to configure OAuth2, skip the authentication: " << e.what() << std::endl;
    }

    Client client(configs.get<std::string>("service_url"), conf);
    std::vector<Producer> producers(num_topics);
    for (int i = 0; i < num_topics; i++) {
        ProducerConfiguration producer_conf;
        producer_conf.setBatchingEnabled(false);
        producer_conf.setBlockIfQueueFull(true);
        producer_conf.setSendTimeout(1000 * 60 * 5);  // 5 minutes
        if (auto result = client.createProducer("my-topic-" + std::to_string(i), producer_conf, producers[i]);
            result != ResultOk) {
            std::cerr << "Failed to create producer " << i << ": " << result << std::endl;
            return 2;
        }
    }

    LOG_INFO("Starting sending messages...");
    asio::thread_pool pool(std::thread::hardware_concurrency());
    std::atomic_bool running{true};
    std::string payload(message_size, 'a');

    std::atomic_int num_producers_created{0};
    for (int i = 0; i < num_topics; i++) {
        ProducerConfiguration producer_conf;
        producer_conf.setBatchingEnabled(false);
        producer_conf.setBlockIfQueueFull(true);
        producer_conf.setSendTimeout(1000 * 60 * 5);  // 5 minutes
        auto topic = "my-topic-" + std::to_string(i);
        client.createProducerAsync(
            topic, producer_conf,
            [i, num_messages, rate, topic, &num_producers_created, &payload, &pool, &running](
                Result result, Producer producer) {
                ++num_producers_created;
                if (result != ResultOk) {
                    std::cerr << "Failed to create producer " << i << ": " << result << std::endl;
                    return;
                }
                asio::post(pool, [&running, &payload, num_messages, producer, rate, topic]() mutable {
                    RateLimiter limiter{rate};
                    for (int i = 0; i < num_messages; i++) {
                        if (!running) {
                            return;
                        }
                        limiter.acquire();
                        auto start = std::chrono::high_resolution_clock::now();
                        producer.sendAsync(
                            MessageBuilder().setContent(payload).build(),
                            [i, start, topic](Result result, const MessageId &) {
                                if (result != ResultOk) {
                                    LOG_ERROR(topic << " Failed to send " << i << ": " << result);
                                    return;
                                }
                                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                   std::chrono::high_resolution_clock::now() - start)
                                                   .count();
                                if (elapsed > 30000) {
                                    LOG_ERROR(topic << " High latency when sending " << i << ": " << elapsed
                                                    << " ms");
                                }
                            });
                    }
                });
            });
    }
    while (num_producers_created < num_topics) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    pool.join();
    client.close();
}
