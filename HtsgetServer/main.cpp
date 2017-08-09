#include <iostream>
#include <utility>
#include <memory>

#define BOOST_ASIO_ENABLE_HANDLER_TRACKING // debugging
// #define NDEBUG

#include "Server.h"

using namespace std;
using namespace asio;
using namespace Http;

int main()
{

    try
    {
        io_service io;
        GenericServer::ServerAddr server_address = make_pair("127.0.0.1", 8888);
        auto app = make_unique<GenericServer>(server_address);

        app->router_.get("/",
                         Handler([](Context &ctx) {
                             cout << "GET /" << endl;
                         }));

        app->router_.get("/home",
                         Handler([](Context &ctx) {
                             cout << "GET /home" << endl;
                         }));

        app->router_.get("/home/level2",
                         Handler([](Context &ctx) {
                             cout << "GET /home/level2" << endl;
                         }));

        app->router_.get("/account/bar",
                         Handler([](Context &ctx) {
                             cout << "GET /account/bar" << endl;
                         }));

        app->router_.get("/account/bar",
                         Handler([](Context &ctx) {
                             cout << "GET /account/bar" << endl;
                         }));

        app->router_.get("/account/foo",
                         Handler([](Context &ctx) {
                             cout << "GET /account/foo" << endl;
                         }));

        app->router_.post("/account/bar",
                          Handler([](Context &ctx) {
                              cout << "POST /account/bar" << endl;
                          }));

        std::cout << app->router_ << std::endl;
        app->run();
    }
    catch (std::exception e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}