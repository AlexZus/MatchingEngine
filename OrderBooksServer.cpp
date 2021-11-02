#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include "OrderBooksProcessor.h"
#include "InputStringParser.h"

using boost::asio::ip::udp;

class OrderBooksServer
{
    static constexpr int MAX_MESSAGE_LENGTH = 10000;

public:
    OrderBooksServer(boost::asio::io_service& io_service, int port)
            : socket(io_service, udp::endpoint(udp::v4(), port))
    {
        start_receive();
    }

private:
    void start_receive()
    {
        socket.async_receive_from(
                boost::asio::buffer(recv_buffer), remote_endpoint,
                boost::bind(&OrderBooksServer::handle_receive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if (error)
        {
            std::cout << "Receive failed: " << error.message() << "\n";
            return;
        }
        orderBookProcessor.feedLines(std::string(recv_buffer.begin(), recv_buffer.begin() + bytes_transferred));
        start_receive();
    }

    udp::socket socket;
    udp::endpoint remote_endpoint;
    boost::array<char, MAX_MESSAGE_LENGTH> recv_buffer;
    OrderBooksProcessor orderBookProcessor;
};

int showUsage()
{
    std::cerr << "Usage: orderBookServer <UDP Port>\n";
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        return showUsage();
    try
    {
        int port = std::atoi(argv[1]);
        boost::asio::io_service io_service;
        OrderBooksServer server(io_service, port);
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
