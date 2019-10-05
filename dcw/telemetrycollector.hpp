#ifndef TELEMETRYCOLLECTOR_H_INCLUDED
#define TELEMETRYCOLLECTOR_H_INCLUDED


#include "./network.hpp"
#include "./macaddress.hpp"
#include "./trafficsorter.hpp" //DataChannelMap

namespace dcw {


struct TelemetryCollector {
  virtual ~TelemetryCollector() = default;

  virtual void Telemetry_OnStationUpdate(const BasicNetwork& network, const MacAddress& primaryMacAddr, const TrafficPolicy::DataChannelMap& dataChannels, const TrafficFilterProfile *trafficFilterProfile) = 0;
  virtual void Telemetry_OnForgetStation(const BasicNetwork& network, const MacAddress& primaryMacAddr) = 0;
  virtual void Telemetry_Clear() = 0;

};

} // namespace dcw

#endif //#ifndef TELEMETRYCOLLECTOR_H_INCLUDED


