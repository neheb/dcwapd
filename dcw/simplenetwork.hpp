#ifndef SIMPLENETWORK_H_INCLUDED
#define SIMPLENETWORK_H_INCLUDED


#include "./network.hpp"

#include <list>
#include <string>

namespace dcw {


//SimpleChannel -- Simple text-only (ssid) implementation of a channel
class SimpleChannel : public BasicChannel {
  const std::string _ssidName;

public:
  SimpleChannel(const char * const ssidName);
  SimpleChannel(const SimpleChannel& rhv);
  explicit SimpleChannel(const BasicChannel& bc);
  ~SimpleChannel() final;

  const char *GetSsidName() const final;
};

//SimpleNetwork -- Simple text-only (ssid) implementation of a network
class SimpleNetwork : public BasicNetwork {
  const SimpleChannel       _primaryChannel;
  std::list<SimpleChannel>  _dataChannels;

public:
  explicit SimpleNetwork(const char * const primarySsidName);
  ~SimpleNetwork() final;
  const BasicChannel& GetPrimaryChannel() const final;
  void GetDataChannels(ChannelSet& output) const final;
  void InsertDataChannel(const char * const ssidName);
};

} // namespace dcw


#endif //#ifndef SIMPLENETWORK_H_INCLUDED
