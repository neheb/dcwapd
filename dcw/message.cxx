
#include "./message.hpp"

#include <cstring>

#include <exception>


namespace {
struct MessageMarshallException : public std::exception {
  virtual const char* what() const noexcept {
    return "Failed to marshall DCW message from buffer!";
  }
};
struct MessageSerializeException : public std::exception {
  virtual const char* what() const noexcept {
    return "Failed to serialize DCW message from buffer!";
  }
};
} // namespace


using namespace dcw;

Message::Message() {
  //
}

Message::Message(const enum dcwmsg_id mid) {
  this->id = mid;
}

Message::Message(const Message& rhv) {
  const auto rhv_m = &rhv;
  auto lhv_m = this;

  std::memcpy(lhv_m, rhv_m, sizeof(struct dcwmsg));
}


void Message::Marshall(const unsigned char * const buf, const unsigned size) {
  if (!dcwmsg_marshal(this, buf, size)) {
    throw MessageMarshallException();
  }
}

unsigned Message::Serialize(unsigned char * const buf, const unsigned size) const {
  unsigned rv = 0;

  rv = dcwmsg_serialize(buf, this, size);
  if (rv == 0) {
    throw MessageSerializeException();
  }

  return rv;
}




