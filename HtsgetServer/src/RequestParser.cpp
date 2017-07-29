#include <iostream>
#include <vector> // emplace_back

#include "RequestParser.h"
#include "Message.h"
#include "Request.h"
#include "Utilities.h" // etoint

namespace Http
{

/*  
        Request         = Request-Line                  ; Section 5.1
                        *(( general-header              ; Section 4.5
                            | request-header            ; Section 5.3
                            | entity-header ) CRLF)     ; Section 7.1
                        CRLF
                        [ message-body ]                ; Section 4.3

        Request-Line   = *(CRLF) Method SP Request-URI SP HTTP-Version CRLF

        Method          = "OPTIONS"                ; Section 9.2
                        | "GET"                    ; Section 9.3
                        | "HEAD"                   ; Section 9.4
                        | "POST"                   ; Section 9.5
                        | "PUT"                    ; Section 9.6
                        | "DELETE"                 ; Section 9.7
                        | "TRACE"                  ; Section 9.8
                        | "CONNECT"                ; Section 9.9
                        | extension-method
        extension-method = token

        Request-URI    = "*" | absoluteURI | abs_path | authority
        https://www.ietf.org/rfc/rfc2396.txt

        HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

        

        Message Headers
        message-header = field-name ":" [ field-value ]
        field-name     = token
        field-value    = *( field-content | LWS )
        field-content  = <the OCTETs making up the field-value
                            and consisting of either *TEXT or combinations
                            of token, separators, and quoted-string>

    */
/*
        Parsing caveats:
            1. did not verify method 
            2. did not verify uri
    */
auto RequestParser::consume(Request &request, char c) -> ParseStatus
{
    using s = RequestParser::State;
    using status = RequestParser::ParseStatus;

    switch (state_)
    {
    case s::req_start:
        if (is_cr(c))
        {
            state_ = s::req_start_lf;
            return status::in_progress;
        }
        if (is_token(c))
        {
            state_ = s::req_method;
            request.method_.push_back(c);
            return status::in_progress;
        }
        return status::reject;
    case s::req_start_lf:
        if (is_lf(c))
        {
            state_ = s::req_start;
            return status::in_progress;
        }
        return status::reject;
    case s::req_method:
        if (is_token(c))
        {
            request.method_.push_back(c);
            return status::in_progress;
        }
        if (is_sp(c))
        {
            state_ = s::req_uri;
            return status::in_progress;
        }
        return status::reject;
    case s::req_uri:
        if (is_uri(c))
        {
            request.uri_.push_back(c);
            return status::in_progress;
        }
        if (is_sp(c))
        {
            state_ = s::req_http_h;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_h:
        if (c == 'H')
        {
            state_ = s::req_http_ht;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_ht:
        if (c == 'T')
        {
            state_ = s::req_http_htt;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_htt:
        if (c == 'T')
        {
            state_ = s::req_http_http;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_http:
        if (c == 'P')
        {
            state_ = s::req_http_slash;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_slash:
        if (c == '/')
        {
            state_ = s::req_http_major;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_major:
        if (is_digit(c))
        {
            request.version_major_ = c - '0';
            state_ = s::req_http_dot;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_dot:
        if (c == '.')
        {
            state_ = s::req_http_minor;
            return status::in_progress;
        }
        return status::reject;
    case s::req_http_minor:
        if (is_digit(c))
        {
            request.version_minor_ = c - '0';
            state_ = s::req_start_line_cr;
            return status::in_progress;
        }
        return status::reject;
    case s::req_start_line_cr:
        if (is_cr(c))
        {
            state_ = s::req_start_line_lf;
            return status::in_progress;
        }
        return status::reject;
    case s::req_start_line_lf:
        if (is_lf(c))
        {
            state_ = s::req_field_name_start;
            return status::in_progress;
        }
        return status::reject;
    case s::req_field_name_start:
        if (is_cr(c))
        {
            state_ = s::req_header_end;
            return status::in_progress;
        }
        if (is_token(c))
        {
            request.headers_.emplace_back();
            request.build_header_name(c);
            state_ = s::req_field_name;
            return status::in_progress;
        }
    case s::req_field_name:
        if (is_token(c))
        {
            request.build_header_name(c);
            return status::in_progress;
        }
        if (c == ':')
        {
            state_ = s::req_field_value;
            return status::in_progress;
        }
        return status::reject;
    case s::req_field_value:
        if (is_sp(c) || is_ht(c))
        {
            return status::in_progress;
        }
        if (is_cr(c))
        {
            state_ = s::req_header_lf;
            return status::in_progress;
        }
        if (!is_ctl(c))
        {
            request.build_header_value(c);
            return status::in_progress;
        }
        return status::reject;
    case s::req_header_lf:
        if (is_lf(c))
        {
            state_ = s::req_header_lws;
            return status::in_progress;
        }
        return status::reject;
    case s::req_header_lws:
        /* 
            LWS            = [CRLF] 1*( SP | HT )
            field-value    = *( field-content | LWS )

            3 branches
                1. c = (SP | HT)
                    encounters \r\n(SP|HT), continue building header value
                2. c = \r
                    encounters \r\n\r, header ended here
                3. c = valid chars 
                    encounters \r\n{c}, starts reading a new header name

        */
        if (is_sp(c) || is_ht(c))
        {
            state_ = s::req_field_value;
            return status::in_progress;
        }
        if (is_cr(c))
        {
            state_ = s::req_header_end;
            return status::in_progress;
        }
        if (is_token(c))
        {
            request.headers_.emplace_back();
            request.build_header_name(c);
            state_ = s::req_field_name;
            return status::in_progress;
        }
        return status::reject;
    case s::req_header_end:
        if (is_lf(c))
        {
            return status::accept;
        }
        return status::reject;
    default:
        break;
    }
    return status::reject;
}

constexpr bool RequestParser::is_char(char c)
{
    return c >= 0 && c <= 127;
}

constexpr bool RequestParser::is_upperalpha(char c)
{
    return c >= 65 && c <= 90;
}

constexpr bool RequestParser::is_loweralpha(char c)
{
    return c >= 97 && c <= 122;
}

constexpr bool RequestParser::is_alpha(char c)
{
    return is_loweralpha(c) || is_upperalpha(c);
}

constexpr bool RequestParser::is_digit(char c)
{
    return c >= 48 && c <= 57;
}

constexpr bool RequestParser::is_ctl(char c)
{
    return (c >= 0 && c <= 31) || c == 127;
}

constexpr bool RequestParser::is_cr(char c)
{
    return c == 13;
}

constexpr bool RequestParser::is_lf(char c)
{
    return c == 10;
}

constexpr bool RequestParser::is_crlf(char c)
{
    return c == 13 || c == 10;
}

constexpr bool RequestParser::is_sp(char c)
{
    return c == 32;
}

constexpr bool RequestParser::is_ht(char c)
{
    return c == 9;
}

constexpr bool RequestParser::is_separator(char c)
{
    switch (c)
    {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case ' ':
    case '\t':
        return true;
    default:
        return false;
    }
}

constexpr bool RequestParser::is_token(char c)
{
    return !is_ctl(c) && !is_separator(c) && is_char(c);
}

constexpr bool RequestParser::is_uri(char c)
{
    for (auto b = std::begin(uri_charset), e = std::end(uri_charset);
         b != e; b++)
    {
        if (c == *b)
        {
            return true;
        }
    }
    return false;
}

auto RequestParser::view_state(RequestParser::State state, RequestParser::ParseStatus status, char c) -> void
{
    std::cout << "state: " << etoint(state)
              << "\tstatus: " << status
              << "\tchar: ";

    if (is_char(c))
    {
        if (is_cr(c))
        {
            std::cout << "\\r";
        }
        else if (is_lf(c))
        {
            std::cout << "\\n";
        }
        else
        {
            std::cout << c;
        }
    }
    else
    {
        std::cout << static_cast<int>(c);
    }

    std::cout << std::endl;
}
}
