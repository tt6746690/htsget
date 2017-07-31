#include <iterator>
#include "catch.hpp"

#include "Message.h"
#include "Utilities.h"

using namespace Http;

TEST_CASE("Message::Manipulate header member", "[Message]")
{
    Message msg;
    REQUIRE(msg.headers_.size() == 0);

    msg.headers_.emplace_back();
    REQUIRE(msg.headers_.size() == 1);

    SECTION("build_header_name")
    {
        msg.build_header_name('k');
        REQUIRE(msg.headers_.back().first == "k");

        msg.build_header_name('e');
        msg.build_header_name('y');
        REQUIRE(msg.headers_.back().first == "key");

        msg.build_header_value('v');
        msg.build_header_value('a');
        REQUIRE(msg.headers_.back().second == "va");

        msg.build_header_value('l');
        msg.build_header_value('u');
        msg.build_header_value('e');
        REQUIRE(msg.headers_.back().second == "value");
    }

    SECTION("manipulate headers")
    {
        msg.header_set("foo", "bar");
        msg.header_set("bar", "baz");
        REQUIRE(msg.headers_.size() == 3);

        SECTION("header_set")
        {
            msg.header_set("foo", "barbar");
            REQUIRE(msg.headers_.size() == 3);

            auto found = std::find_if(msg.headers_.begin(), msg.headers_.end(), [](auto &header) {
                return header.first == "foo";
            });
            REQUIRE(found != msg.headers_.end());
            REQUIRE(found->second == "barbar");
        }

        SECTION("header_unset")
        {
            msg.header_unset("foo");
            REQUIRE(msg.headers_.size() == 2);

            auto found = std::find_if(msg.headers_.begin(), msg.headers_.end(), [](auto &header) {
                return header.first == "foo";
            });
            REQUIRE(found == msg.headers_.end());
        }
    }
}
