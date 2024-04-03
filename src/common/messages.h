#pragma once

#include <memory>
#include <tuple>
namespace common {
namespace rpc {

// Assume messages max of 1KB
constexpr int REQUEST_BUF_MAX = 1024;

enum class RequestType {
  Register = 1,
  TaskUpdate,
};
enum class NodeType { Master, Worker, Client };

class Request {
  /// Format:
  /// {
  ///   request type,
  ///   sender type,
  ///   request payload
  /// }
  ///
public:
  void SetType(RequestType rt) noexcept;
  void SetSender(NodeType nt) noexcept;
  void SetData(std::unique_ptr<char> data, int size) noexcept;

  int Size() const noexcept;

  /**
   * @brief Write this object as a bytestring
   *
   * @param buffer bytestring destination
   * @param bufsize length of the bytestring buffer. Should be large enough to
   * contain object serialization.
   * @return n bytes written
   */
  int Marshall(char *buffer, int bufsize) const;

  /**
   * @brief Initialize this object from a bytestring
   *
   * @param buffer bytestring of object data
   * @param bufsize length of the bytestring
   */
  void Unmarshall(std::unique_ptr<char> buffer, int bufsize) const;

private:
  RequestType type;
  NodeType sender;
  std::unique_ptr<char> data;
  int data_len;
};

} // namespace rpc

} // namespace common
