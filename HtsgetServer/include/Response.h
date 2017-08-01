#ifndef RESPONSE_H
#define RESPONSE_H

#include <array>
#include "asio.hpp"

#include "Message.h"
#include "Utilities.h"
#include "Constants.h"

namespace Http
{

class Response : public Message
{
public:
  using BuildStatus = int;

  /**
   * @brief   Generates response string
   */
  auto
  to_payload() const -> std::string;

  /**
   * @brief   gets/Sets status code for response
   */
  auto status_code() -> StatusCode;
  void status_code(StatusCode status_code);

  /**
   * @brief   Gets reason phrase for this instance
   */
  auto reason_phrase() -> std::string;
  /**
   * @brief   Gets status line 
   * 
   * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
   */
  auto status_line() const -> std::string;
  auto static to_status_line(
      StatusCode status_code,
      int http_version_major = 1,
      int http_version_minor = 1) -> std::string;

  /**
   * @brief   Write to body
   * 
   * Types: 
   *    string
   *        sets content-type to text/html or text/plain
   *        sets content-length
   *    buffer, 
   *        sets content-type to application/octet-stream
   *        sets content-length
   *    stream, 
   *        sets content-type to application/octet-stream
   *    json, 
   *        sets content-type to application/json
   */
  void
  write(std::string string);

private:
  StatusCode status_code_ = StatusCode::OK; // defaults to 200 OK

public:
  /**
   * @brief   Gets numeric status code given StatusCode
   */
  static auto constexpr status_code_to_int(StatusCode status_code) -> int
  {
    return enum_map(status_codes, status_code);
  }
  /**
   * @brief   Gets reason phrase given StatusCode
   */
  static auto constexpr status_code_to_reason(StatusCode status_code) -> char *
  {
    return enum_map(reason_phrases, status_code);
  }

  friend std::ostream &operator<<(std::ostream &strm, const Response &response);
};
}

#endif