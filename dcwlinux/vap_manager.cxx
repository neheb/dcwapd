

#include "./vap_manager.hpp"
#include "./virtual_ap.hpp"

#include "dcw/dcwlog.hpp"
#include "dcw/devicepolicy.hpp"

#include <cstring>

#include <exception>

namespace {
struct VAPNotFoundException : public std::exception {
  virtual const char* what() const throw() {
    return "VAP Not Found";
  }
};
struct VAPAlreadyExistsException : public std::exception {
  virtual const char* what() const throw() {
    return "VAP Already Exists";
  }
};
struct VAPAllocationFailedException : public std::exception {
  virtual const char* what() const throw() {
    return "VAP Allocation Failed";
  }
};
} // namespace

using namespace dcwlinux;

VAPManager::VAPManager() {
  //
}

VAPManager::~VAPManager() {
  for (auto i = _vaps.begin(); i != _vaps.end(); i++) {
    delete(*i);
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
  for (auto i = _vaps.begin(); i != _vaps.end(); i++) {
    if (strcmp((*i)->GetPrimaryChannel().GetSsidName(), primarySsidName) == 0) {
      throw VAPAlreadyExistsException();
    }
  }

  //then instanciate it...
  auto  const vap = new VirtualAP(
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
  for (auto i = _vaps.begin(); i != _vaps.end(); i++) {
    if (strcmp((*i)->GetPrimaryChannel().GetSsidName(), primarySsidName) == 0) {
      return *(*i);
    }
  }
  throw VAPNotFoundException();
}

void DestroyVAP(const char * const primarySsidName) {
  //
}

void VAPManager::SetAllTelemetryCollector(::dcw::TelemetryCollector * const tc) {
  for (auto i = _vaps.begin(); i != _vaps.end(); i++) {
    (*i)->SetTelemetryCollector(tc);
  }
}

