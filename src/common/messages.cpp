#include "messages.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <utility>

namespace common {
namespace rpc {

Request::Request()
    : type(RequestType::Invalid), sender(NodeType::Invalid), data(),
      data_len(0) {}

Request::Request(const Request &other) {
  this->type = other.type;
  this->sender = other.sender;
  this->data_len = other.data_len;

  char *buf = new char[other.data_len];
  memcpy(buf, other.data.get(), other.data_len);
  this->data.reset(buf);
}
Request::~Request() {}

void Request::SetType(RequestType rt) noexcept { this->type = rt; }
void Request::SetSender(NodeType nt) noexcept { this->sender = nt; }
void Request::SetData(std::unique_ptr<char> data, int size) noexcept {
  this->data = std::move(data);
  this->data_len = size;
}
RequestType Request::GetType() const noexcept { return this->type; }
NodeType Request::GetSender() const noexcept { return this->sender; }

const std::unique_ptr<char> &Request::GetData() const noexcept {
  return const_cast<std::unique_ptr<char> &>(this->data);
}

int Request::HeaderSize() const noexcept {
  return sizeof(this->type) + sizeof(this->sender) + sizeof(this->data_len);
}
int Request::DataSize() const noexcept { return this->data_len; }
int Request::Size() const noexcept {
  return this->HeaderSize() + this->DataSize();
}

int Request::Marshall(char *buffer, int bufsize) const {
  // Serialization Order:
  // | Request type | Sender type | Len(data segment) | data |
  std::cerr << "RequestMarshall: bufsize=" << bufsize
            << " this->size=" << this->Size() << "\n";
  assert(this->Size() <= bufsize);
  memset(buffer, 0, bufsize);
  int offset = 0;
  int n_type = htonl(static_cast<int>(this->type));
  int n_sender = htonl(static_cast<int>(this->sender));
  int n_datalen = htonl(this->data_len);

  memcpy(buffer + offset, &n_type, sizeof(n_type));
  offset += sizeof(n_type);
  memcpy(buffer + offset, &n_sender, sizeof(n_sender));
  offset += sizeof(n_sender);
  memcpy(buffer + offset, &n_datalen, sizeof(n_datalen));
  offset += sizeof(n_datalen);
  memcpy(buffer + offset, this->data.get(), n_datalen);
  offset += this->data_len;

  return offset;
}

void Request::UnmarshallHeaders(const char *buffer, int bufsize) noexcept {
  std::cout << "Request::UnmarshallHeaders\n";
  int offset = 0;
  int n_type = -1;
  int n_sender = -1;
  int n_datalen = -1;

  // Load serialization
  memcpy(&n_type, buffer, sizeof(n_type));
  offset += sizeof(n_type);
  memcpy(&n_sender, buffer + offset, sizeof(n_sender));
  offset += sizeof(n_sender);
  memcpy(&n_datalen, buffer + offset, sizeof(n_datalen));
  offset += sizeof(n_datalen);

  // Initialization
  this->type = static_cast<RequestType>(ntohl(n_type));
  this->sender = static_cast<NodeType>(ntohl(n_sender));
  this->data_len = ntohl(n_datalen);
}

void Request::UnmarshallData(const char *buffer, int bufsize) noexcept {
  std::cout << "Request::UnmarshallData\n";
  // No data expected
  if (this->data_len < 1) {
    return;
  }
  assert(bufsize == this->data_len);

  this->data.reset();
  this->data = std::make_unique<char>(this->data_len);
  memcpy(this->data.get(), buffer, this->data_len);
}

} // namespace rpc
} // namespace common
