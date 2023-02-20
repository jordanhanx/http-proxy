#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>  

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 5 && argc != 6)
        {
            std::cerr <<
                "Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    http-client-sync get www.example.com 80 /\n" <<
                "    http-client-sync post www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const method = argv[1];
        auto const host = argv[2];
        auto const port = argv[3];
        auto const target = argv[4];
        int version = argc == 6 && !std::strcmp("1.0", argv[5]) ? 10 : 11;

        //print receiving a new request
        auto curr_time = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(curr_time);
        std::cout << "\"" <<  method <<  " " << host << " HTTP/1.1\" from " << std::ctime(&end_time) << std::endl;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);
        http::request<http::string_body> req;
        // Set up an HTTP GET request message
        if(beast::iequals(method, "get")){
            req = {http::verb::get, target, version};
        }else if(beast::iequals(method, "post")){
            req = {http::verb::post, target, version};
        }

        // http::request<http::string_body> req{http::verb::get, target, version};
        
                    
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        
        std::cout << "Requesting \"" <<  method <<  " " << host << " HTTP/1.1\" from " << host << std::endl;
        // Send the HTTP request to the remote host
        http::write(stream, req);
        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;
        // http::response_parser<http::string_body> parser;
        // http::read_header(stream, buffer, parser);
        // Declare a container to hold the response
        http::response<http::dynamic_body> res;
        // Receive the HTTP response
        http::read(stream, buffer, res);

        // * get method
        if(beast::iequals(method, "get")){
            // Write the message to standard out
            std::cout << "\nFor test: \n" << res.base() << std::endl;
            std::cout <<res.base()[http::field::last_modified] << std::endl;
            // http::read(stream, buffer, parser);
            std::cout << "Received \"HTTP/1.1 " << res.base().result_int() << " " << res.base().result() <<  "\" from " << host << std::endl;
        }
        
        // * post method
        if(beast::iequals(method, "post")){
            // write an empty body response 
            http::response<http::empty_body> response;
            response.version(11);
            response.result(http::status::ok);
            // response.set(http::field::server, res.base()[http::field::server]);
            response.set(http::field::server, host);
            http::write(stream, response);
            std::cout << "Received \"HTTP/1.1 " << response.base().result_int() << " " << response.base().result() <<  "\" from " << host << std::endl;
            std::cout << "\nFor test: \n"  << response << std::endl;
        }

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "400" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}