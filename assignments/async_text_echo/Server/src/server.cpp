#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, boost::asio::io_context& io_context)
        : socket_(std::move(socket)), strand_(boost::asio::make_strand(io_context)) {}

    void start() {
        do_read();
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
                        std::cout << "[Client]: " << line << std::endl;
                        std::string reply = "Re:" + line + "\n";
                        self->do_write(reply);
                    }
                }
            )
        );
    }

    void do_write(const std::string& msg) {
        auto buf = std::make_shared<std::string>(msg);
        auto self = shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(*buf),
            boost::asio::bind_executor(strand_,
                [self, buf, this](boost::system::error_code, std::size_t) {
                    self->do_read();
                }
            )
        );
    }

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::shared_ptr<boost::asio::streambuf> buf_;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, unsigned short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), io_context_(io_context) {
        do_accept();
    }
private:
    void do_accept() {
        acceptor_.async_accept(
            boost::asio::make_strand(io_context_),
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), io_context_)->start();
                }
                do_accept();
            }
        );
    }
    tcp::acceptor acceptor_;
    boost::asio::io_context& io_context_;
};

int main() {
    boost::asio::io_context io_context;
    Server server(io_context, 5555);
    std::vector<std::thread> threads;
    unsigned int n = std::thread::hardware_concurrency();
    for (unsigned int i = 0; i < n; ++i) {
        threads.emplace_back([&io_context]() { io_context.run(); });
    }
    for (auto& t : threads) t.join();
    return 0;
}
