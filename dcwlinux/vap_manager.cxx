

#include "./vap_manager.hpp"
#include "./virtual_ap.hpp"

#include "dcw/dcwlog.hpp"
#include "dcw/devicepolicy.hpp"

#include <cstring>

#include <exception>

namespace {
struct VAPNotFoundException : public std::exception {
  const char* what() const noexcept override {
    return "VAP Not Found";
  }
};
struct VAPAlreadyExistsException : public std::exception {
  const char* what() const noexcept override {
    return "VAP Already Exists";
  }
};
struct VAPAllocationFailedException : public std::exception {
  const char* what() const noexcept override {
    return "VAP Allocation Failed";
  }
};
} // namespace

using namespace dcwlinux;

VAPManager::VAPManager() {
  //
}

VAPManager::~VAPManager() {
  for (const auto &vap : _vaps) {
    delete(vap);
  }
  _vaps.clear(); //defensive...
}

VirtualAP& VAPManager::InstanciateVAP(
  const char * const primarySsidName,
  const char * const primarySsidIfName,
  const ::dcw::DevicePolicy& devicePolicy,
  ::dcw::TrafficSorter& trafficSorter,
  ::dcw::EventReactor& eventReactor) {

  //first ensure the VAP dont already exist...
  for (const auto &vap : _vaps) {
    if (strcmp(vap->GetPrimaryChannel().GetSsidName(), primarySsidName) == 0) {
      throw VAPAlreadyExistsException();
    }
  }

  //then instanciate it...
  const auto vap = new VirtualAP(
    primarySsidName,
    primarySsidIfName,
    devicePolicy,
    trafficSorter,
    eventReactor
  );
  if (vap == NULL) {
    throw VAPAllocationFailedException();
  }

  //remember it for later...
  _vaps.insert(vap);

  return *vap;
}

VirtualAP& VAPManager::operator[](const char * const primarySsidName) const {
  for (const auto &vap : _vaps) {
    if (strcmp(vap->GetPrimaryChannel().GetSsidName(), primarySsidName) == 0) {
      return *vap;
    }
  }
  throw VAPNotFoundException();
}

void DestroyVAP(const char * const primarySsidName) {
  //
}

void VAPManager::SetAllTelemetryCollector(::dcw::TelemetryCollector * const tc) {
  for (const auto &vap : _vaps) {
    vap->SetTelemetryCollector(tc);
  }
}

