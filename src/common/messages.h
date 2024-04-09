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

/**
 * @class Request
 * @brief An RPC request. Can be marshalled to a bytestream.
 *
 * Example:
 *
 * Request r;
 * char buffer[req.HeaderSize() + 0]; // '0' would be payload size
 * r.SetType(RequestType::Register);
 * r.SetSender(NodeType::Worker);
 * r.Marshall(buffer, len(buffer))
 *
 */
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
  /**
   * @brief Default constructor
   */
  Request() noexcept;

  /**
   * @brief Copy constructor.
   *
   * Creates a new copy of other->data, without freeing other's payload.
   *
   * @param other request to copy from
   */
  Request(const Request &other);

  /**
   * @brief dtor
   */
  ~Request();

  /**
   * @brief Update the request type id
   *
   * @param rt type of RPC request
   */
  void SetType(RequestType rt) noexcept;

  /**
   * @brief Update the sender type
   *
   * @param nt type of node sending request
   */
  void SetSender(NodeType nt) noexcept;

  /**
   * @brief Update the request payload
   *
   * @param data payload data
   * @param size payload size in bytes
   */
  void SetData(std::unique_ptr<char> data, int size) noexcept;

  /**
   * @brief RPC request type
   *
   * @return current set request type
   */
  RequestType GetType() const noexcept;

  /**
   * @brief Sending node type
   *
   * @return currently set node type
   */
  NodeType GetSender() const noexcept;

  /**
   * @brief Get an immutable reference to the request payload
   */
  const std::unique_ptr<char> &GetData() const noexcept;

  /**
   * @brief Total size
   *
   * Equal to this->HeaderSize() + this->DataSize()
   * @return full marshalled packet size in bytes
   */
  int Size() const noexcept;

  /**
   * @brief Get the payload size
   *
   * @return payload size in bytes
   */
  int DataSize() const noexcept;

  /**
   * @brief Size of the headers. Always fixed.
   *
   * @return header size in bytes
   */
  constexpr int HeaderSize() const noexcept {
    return sizeof(this->type) + sizeof(this->sender) + sizeof(this->data_len);
  }

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

  inline void Reset() noexcept {
    this->type = RequestType::Invalid;
    this->sender = NodeType::Invalid;
    this->data.reset();
    this->data_len = 0;
  }

private:
  RequestType type;           // Request type identifier
  NodeType sender;            // Sending node's type
  std::unique_ptr<char> data; // Request payload
  int data_len;               // Size of request payload in bytes
};

} // namespace rpc

} // namespace common
