#include <iostream>
#include <cassert>
#include <string>
#include <utility>
#include <algorithm>

#include "Message.h"

namespace Http
{

void Message::build_header_name(char c)
{
    assert(headers_.size() != 0);
    header_name(headers_.back()).push_back(c);
}
void Message::build_header_value(char c)
{
    assert(headers_.size() != 0);
    header_value(headers_.back()).push_back(c);
}

auto Message::header_name(HeaderType &header) -> Message::HeaderNameType &
{
    return std::get<0>(header);
}
auto Message::header_value(HeaderType &header) -> Message::HeaderValueType &
{
    return std::get<1>(header);
}

void Message::header_set(HeaderNameType name, HeaderValueType value)
{
    auto header = std::make_pair(name, value);
    auto found = find_if(headers_.begin(), headers_.end(),
                         [&](auto &header) {
                             return header.first == name;
                         });
    if (found != headers_.end())
        *found = header;
    else
        headers_.push_back(header);
}

void Message::header_unset(HeaderNameType name)
{
    auto end = std::remove_if(headers_.begin(), headers_.end(),
                              [&](auto &header) {
                                  return header.first == name;
                              });
    headers_.erase(end, headers_.end());
}

auto Message::version(int major, int minor) -> std::string
{
    return std::to_string(major) + "." + std::to_string(minor);
}
}