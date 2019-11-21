#ifndef MESSAGESOCKET_H_INCLUDED
#define MESSAGESOCKET_H_INCLUDED

#include "./eventreactor.hpp"

namespace dcw {

struct Message;
struct MacAddress;

struct MessageSocket : public virtual EventReactor::IOProvider {
  ~MessageSocket() override = default;
  virtual void ReceiveMessage(MacAddress& source, Message& msg) = 0;
  virtual void TransmitMessage(const MacAddress& dest, const Message& msg) = 0;
  virtual void GetSourceMacAddress(MacAddress& source) = 0;
};

} // namespace dcw

#endif //#define MESSAGESOCKET_H_INCLUDED
