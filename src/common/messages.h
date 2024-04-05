#pragma once

#include <memory>
namespace common {
namespace rpc {

// Assume messages max of 1KB
constexpr int REQUEST_BUF_MAX = 2048;

enum class RequestType {
  Invalid = -1,
  Register = 1, // A node introducing itself to the master
  TaskUpdate,
};
enum class NodeType {
  Invalid = -1, // Error value
  Master = 0,   // Node is a MapReduce master
  Worker = 1,   // Node is a MapReduce worker
  Client = 2,   // Node is a user program
};

class Request {
  /// Format:
  /// {
  ///   request size,
  ///   request type,
  ///   sender type,
  ///   request payload
  /// }
  ///
public:
  Request();
  Request(const Request &other);
  ~Request();

  void SetType(RequestType rt) noexcept;
  void SetSender(NodeType nt) noexcept;
  void SetData(std::unique_ptr<char> data, int size) noexcept;
  RequestType GetType() const noexcept;
  NodeType GetSender() const noexcept;
  const std::unique_ptr<char> &GetData() const noexcept;

  int DataSize() const noexcept;
  int HeaderSize() const noexcept;
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
   * @brief Initialize this object's payload from bytestring
   *
   * @param buffer bytestring of object data
   * @param bufsize length of the bytestring
   */
  void UnmarshallData(const char *buffer, int bufsize) noexcept;

  /**
   * @brief Initialize object's metadata from bytestring
   *
   * @param buffer bytestring of object data
   * @param bufsize length of the bytestring
   */
  void UnmarshallHeaders(const char *buffer, int bufsize) noexcept;

private:
  RequestType type;
  NodeType sender;
  std::unique_ptr<char> data;
  int data_len;
};

} // namespace rpc

} // namespace common
