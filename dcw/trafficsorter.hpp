
#ifndef TRAFFICSORTER_H_INCLUDED
#define TRAFFICSORTER_H_INCLUDED

#include "./macaddress.hpp"
#include "./network.hpp"
#include "./trafficfilterprofile.hpp"

#include <map>

namespace dcw {

struct TrafficPolicy {
  typedef std::map< ::dcw::MacAddress, const ::dcw::BasicChannel*> DataChannelMap;
  //               ^ client DC macaddr      ^ AP ssid

  DataChannelMap                dataChannels;
  const TrafficFilterProfile   *trafficFilterProfile;
  TrafficPolicy() : trafficFilterProfile(NULL) {}
};

struct TrafficSorter {
  virtual ~TrafficSorter() = default;

  virtual void ApplyClientTrafficPolicy(const MacAddress& primaryAddr, const TrafficPolicy& policy) = 0;
  virtual void RemoveClientTrafficPolicy(const MacAddress& primaryAddr) = 0;
};

} // namespace dcw


#endif // #ifndef TRAFFICSORTER_H_INCLUDED
