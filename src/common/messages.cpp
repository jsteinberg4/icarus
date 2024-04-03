#include "messages.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
namespace common {
namespace rpc {
Request::Request()
    : type(RequestType::Invalid), sender(NodeType::Invalid), data(),
      data_len(0) {}
Request::~Request() {}

void Request::SetType(RequestType rt) noexcept { this->type = rt; }
void Request::SetSender(NodeType nt) noexcept { this->sender = nt; }
void Request::SetData(std::unique_ptr<char> data, int size) noexcept {
  this->data = std::move(data);
  this->data_len = size;
}
RequestType Request::GetType() const noexcept { return this->type; }
NodeType Request::GetSender() const noexcept { return this->sender; }

std::pair<const std::unique_ptr<char> &, const int &>
Request::GetData() const noexcept {
  return std::tie<const std::unique_ptr<char> &, const int &>(this->data,
                                                              this->data_len);
}

int Request::Size() const noexcept {
  int size = sizeof(this->type) + sizeof(this->sender) +
             sizeof(this->data_len) + data_len;
  std::cout << "Request::Size: size=" << size << "\n";
  return size;
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

  memcpy(buffer, &n_type, sizeof(n_type));
  offset += sizeof(n_type);
  memcpy(buffer + offset, &n_sender, sizeof(n_sender));
  offset += sizeof(n_sender);
  memcpy(buffer + offset, &n_datalen, sizeof(n_datalen));
  offset += sizeof(n_datalen);
  memcpy(buffer + offset, this->data.get(), n_datalen);
  offset += n_datalen;

  return offset;
}

void Request::Unmarshall(const char *buffer, int bufsize) {
  std::cout << "Request::Unmarshall\n";
  int offset = 0;
  int n_type = -1;
  int n_sender = -1;
  int n_datalen = -1, h_datalen = -1;
  char *data_buf;

  // Load serialization
  memcpy(&n_type, buffer, sizeof(n_type));
  offset += sizeof(n_type);
  memcpy(&n_sender, buffer + offset, sizeof(n_sender));
  offset += sizeof(n_sender);
  memcpy(&n_datalen, buffer + offset, sizeof(n_datalen));
  offset += sizeof(n_datalen);

  // Load data segment (dynamic length)
  h_datalen = ntohl(n_datalen);
  data_buf = new char[h_datalen];
  memcpy(data_buf, buffer + offset, h_datalen);
  offset += h_datalen;

  // Initialization
  this->type = static_cast<RequestType>(ntohl(n_type));
  this->sender = static_cast<NodeType>(ntohl(n_sender));
  this->data_len = h_datalen;
  this->data.reset(data_buf);
}

} // namespace rpc
} // namespace common
