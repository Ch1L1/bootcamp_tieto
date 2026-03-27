#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using boost::asio::ip::tcp;


class Client : public std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_context& io_context, const std::string& host, unsigned short port)
        : io_context_(io_context), socket_(io_context), strand_(boost::asio::make_strand(io_context)), host_(host), port_(port) {}

    void start() {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));
        auto self = shared_from_this();
        boost::asio::async_connect(socket_, endpoints,
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, const tcp::endpoint&) {
                    if (!ec) self->do_read();
                }
            )
        );
    }

    void send(const std::string& msg) {
        auto buf = std::make_shared<std::string>(msg);
        auto self = shared_from_this();
        boost::asio::post(strand_, [self, buf]() {
            boost::asio::async_write(self->socket_, boost::asio::buffer(*buf),
                boost::asio::bind_executor(self->strand_,
                    [self, buf](boost::system::error_code, std::size_t) {
                    }
                )
            );
        });
    }

private:
    void do_read() {
        buf_ = std::make_shared<boost::asio::streambuf>();
        auto self = shared_from_this();
        boost::asio::async_read_until(socket_, *buf_, '\n',
            boost::asio::bind_executor(strand_,
                [self, this](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        std::istream is(self->buf_.get());
                        std::string line;
                        std::getline(is, line);
                        std::cout << "[Server]: " << line << std::endl;
                        self->do_read();
                    }
                }
            )
        );
    }

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::shared_ptr<boost::asio::streambuf> buf_;
    std::string host_;
    unsigned short port_;
};

int main() {
    boost::asio::io_context io_context;
    auto client = std::make_shared<Client>(io_context, "127.0.0.1", 5555);
    client->start();
    std::thread net_thread([&io_context]() { io_context.run(); });

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty())
            client->send(line + "\n");
    }

    io_context.stop();
    net_thread.join();
    return 0;
}
