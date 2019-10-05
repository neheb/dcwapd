
#include "./ap_configuration.hpp"
#include "./macremapper_driver.hpp"
#include "./vap_manager.hpp"
#include "./virtual_ap.hpp"

#include "dcw/dcwlog.hpp"


#include <exception>

namespace {
struct ReloadRunningConfigurationNotImplementedException : public std::exception {
  const char* what() const noexcept override {
    return "ReloadRunningConfiguration() Not Yet Implemented";
  }
};
struct ValidationFailureException : public std::exception {
  const char* what() const noexcept override {
    return "Failed to validate configuration!";
  }
};
} // namespace



using namespace dcwlinux;

static const char _defaultProfileName[] = "TFP_Default";

APConfiguration::APConfiguration(const APConfigurationProvider& initialConfiguration) {
  try {
    LoadConfiguration(initialConfiguration);
  }
  catch (...) {
    Cleanup();
    throw;
  }
}

APConfiguration::~APConfiguration() {
  Cleanup();
}

void APConfiguration::Apply(MacRemapperDriver& driver, VAPManager& vapman, ::dcw::EventReactor& eventReactor) const {
  //load the traffic filter profiles into the driver...
  for (const auto &_trafficFilterProfile : _trafficFilterProfiles) {
    driver.ParseAndLoadFilter(*_trafficFilterProfile.second); //XXX defensive check for NULL?
  }

  //instanciate all the virtual APs... (one per primary SSID)
  for (const auto &data : _primaryDataMap) {
    const auto pssidName = data.first.c_str();
    const auto ifnameIter = _ssidIfnameMap.find(pssidName);
    if (ifnameIter == _ssidIfnameMap.end()) {
      //defensive
      dcwlogerrf("For whatever reason primary SSID '%s' dont have an interface name...\n", pssidName);
      throw ValidationFailureException();
    }

    //instanciate the actual VirtualAP object...
    auto &vap = vapman.InstanciateVAP(
      pssidName,                    //primarySsidName
      ifnameIter->second.c_str(),   //primarySsidIfName
      *this,                        //devicePolicy
      driver,                       //trafficSorter
      eventReactor                  //eventReactor
    );

    //give it its data channels...
    for (const auto &channel : data.second) {
      const auto dssidName = channel.c_str();

      //determine the data channel interface name (if any)
      auto dssidIfname = static_cast<const char *>(NULL); //note: this may be null...
      const auto difnameIter = _ssidIfnameMap.find(dssidName);
      if (difnameIter != _ssidIfnameMap.end()) dssidIfname = difnameIter->second.c_str();

      //XXX should eventually use the proper functions...
      //XXX InsertDataChannel() comes from "VirtualAP" base class "Brctlnetwork"
      vap.InsertDataChannel(dssidName, dssidIfname);
    }
  }
}

void APConfiguration::ReloadRunningConfiguration(const APConfigurationProvider& newConfiguration) {
  throw ReloadRunningConfigurationNotImplementedException();
}

void APConfiguration::Dump() const {
  dcwlogdbgf("%s\n", "AP Configuration Dump:");

  dcwlogdbgf("%s\n", "  Traffic Filter Profiles:");
  for (const auto &profile : _trafficFilterProfiles) {
    dcwlogdbgf("    %s\n", profile.second->GetName());
  }

  dcwlogdbgf("%s\n", "  SSIDs:");
  for (const auto &primary : _primaryDataMap) {
    dcwlogdbgf("    Primary '%s'\n", primary.first.c_str());
    for (const auto &data : primary.second) {
      dcwlogdbgf("      Data '%s'\n", data.c_str());
    }
  }

  dcwlogdbgf("%s\n", "  SSID Interfaces:");
  for (const auto &ifname : _ssidIfnameMap) {
    dcwlogdbgf("    '%s' -> '%s'\n", ifname.first.c_str(), ifname.second.c_str());
  }

  dcwlogdbgf("%s\n", "  Station Traffic Filter Profiles:");
  for (const auto &filter : _stationFilterProfiles) {
    dcwlogdbgf("    '%s' -> '%s'\n", filter.first.ToString().c_str(), filter.second->GetName());
  }
}

const dcw::TrafficFilterProfile& APConfiguration::GetTrafficFilterProfile(const dcw::MacAddress& device) const {
  //first lookup the TFP name we intend to use for this device...
  auto stationTfp = _stationFilterProfiles.find(device);
  if (stationTfp == _stationFilterProfiles.end()) {
    //lookup failed... use the default traffic filter profile for this device...
    dcwlogdbgf("Defaulting device %s to default profile\n", device.ToString().c_str());
    const auto defaultCftfp = _trafficFilterProfiles.find(_defaultProfileName);
    if (defaultCftfp == _trafficFilterProfiles.end()) {
      //defensive...
      dcwlogerrf("%s\n", "Unable to lookup traffic default filter profile");
      throw ValidationFailureException();
    }
    if (defaultCftfp->second == NULL) {
      //defensive... (since we are dereferencing to return a reference)
      dcwlogerrf("%s\n", "Default traffic filter profile is NULL");
      throw ValidationFailureException();
    }
    return *defaultCftfp->second;
  }

  if (stationTfp->second == NULL) {
    //defensive... (since we are dereferencing to return a reference)
    dcwlogerrf("Traffic filter profile for device '%s' is NULL\n", stationTfp->first.ToString().c_str());
    throw ValidationFailureException();
  }

  return *stationTfp->second;
}

void APConfiguration::FilterPermittedDataChannels(const dcw::MacAddress& device, const unsigned deviceTotalCapableDataChannels, dcw::BasicNetwork::ChannelSet& allowedDataChannels) const {
  //for now, do nothing...
  //(let the station have all data channels)

  //note: it is only advised to do this for load-balancing purposes
  //      the optimal solution is to offer the station all available
  //      data-channels, then let the stations detemine which is the 
  //      best setup
  //
  //in the future though, this may be used for load-balancing...
  //here we could algorithmically spread-out the clients on
  //different data-channels if we have many

}

void APConfiguration::LoadConfiguration(const APConfigurationProvider& conf) {
  //read in the "C FILE" compatible traffic filter profiles...
  //remember, it is OUR (the called or InstanciateCFileTrafficFilterProfiles()) to delete these...
  APConfigurationProvider::CFTFPList tfps;
  conf.InstanciateCFileTrafficFilterProfiles(tfps);
  for (const auto &tfp : tfps) {
    if (tfp == NULL) {
      dcwlogwarnf("%s\n", "A NULL traffic filter profile was detected!");
      continue;
    }
    if (_trafficFilterProfiles.find(tfp->GetName()) != _trafficFilterProfiles.end()) {
      dcwlogwarnf("Ignoring existing traffic filter profile: %s\n", tfp->GetName());
      delete tfp;
      continue;
    }
    _trafficFilterProfiles[tfp->GetName()] = tfp;
  }

  //read in the primary SSIDs...
  SsidSet primarySsids;
  conf.GetPrimarySsids(primarySsids); //XXX... is it a good idea to load directly here? (as opposed to a seperate set declared here on the stack)

  //load each primary SSID's configuration...
  for (const auto &primarySsid : primarySsids) {
    const auto pssidName = primarySsid.c_str();

    //create an entry in the primary data map...
    //note: not really necessary, but helps with
    //      detecting primary channels with no
    //      data SSIDs on validation...
    _primaryDataMap[pssidName];

    //get the interface name (ifconfig) used for the primary ssid...
    const auto pssidIfName = conf.GetSsidIfname(pssidName);
    if (pssidIfName == NULL) {
      dcwlogerrf("No interface provided for primary SSID: %s\n", pssidName);
      throw ValidationFailureException();
    }
    if (pssidIfName[0] == '\0') {
      dcwlogerrf("Empty interface provided for primary SSID: %s\n", pssidName);
      throw ValidationFailureException();
    }
    _ssidIfnameMap[pssidName] = pssidIfName;

    //get the associated data channels for this primary ssid...
    SsidSet dataSsids;
    conf.GetDataSsids(dataSsids, pssidName);
    for (const auto &dataSsid : dataSsids) {
      const auto dssidName = dataSsid.c_str();

      //get the data channel's interface name (if any)
      const auto dssidIfName = conf.GetSsidIfname(dssidName);
      if (dssidIfName != NULL) _ssidIfnameMap[dssidName] = dssidIfName;

      //add this as a data channel ssid to the primary ssid...
      _primaryDataMap[pssidName].insert(dssidName);
    }
  }

  //read in station TFPs...
  APConfigurationProvider::StationTFPMap stationTfpMap;
  conf.GetStationTrafficFilterProfiles(stationTfpMap);
  for (auto i = stationTfpMap.begin(); i != stationTfpMap.end(); i++) {
    const auto cftfpIter = _trafficFilterProfiles.find(i->second);
    if (cftfpIter == _trafficFilterProfiles.end()) {
      dcwlogerrf("Invalid traffic filter profile name '%s' for MAC address '%s'\n", i->second.c_str(), i->first.ToString().c_str());
      throw ValidationFailureException();
    }
    _stationFilterProfiles[i->first] = cftfpIter->second;
  }

  //one we have read everything in, validate it...
  Dump();
  SelfValidate();
}

void APConfiguration::Cleanup() {
  //delete any instanciated traffic filter profiles...
  for (const auto &trafficFilterProfile : _trafficFilterProfiles) {
    delete trafficFilterProfile.second;
  }
  _trafficFilterProfiles.clear();
}


void APConfiguration::SelfValidate() const {

  for (const auto &pdc : _primaryDataMap) {
    const auto pssidName = pdc.first.c_str();

    //ensure each primary SSID has at least one data channel
    if (pdc.second.empty()) {
      dcwlogerrf("Configured primary SSID \"%s\" has no associated data channels\n", pssidName);
      throw ValidationFailureException();
    }

    //validate the associated data channels:
    for (const auto &dssid : pdc.second) {
      const auto dssidName = dssid.c_str();

      //ensure the data ssid name is NOT used as a primary...
      if (_primaryDataMap.find(dssidName) != _primaryDataMap.end()) {
        dcwlogerrf("Configured primary SSID \"%s\" declares primary SSID \"%s\" as data SSID.\n", pssidName, dssidName);
        throw ValidationFailureException();
      }

      //warn for each data SSID that does not have an associated network interface
      auto sifnIter = _ssidIfnameMap.find(dssidName);
      if (sifnIter == _ssidIfnameMap.end()) {
        dcwlogwarnf("Configured data SSID \"%s\" for primary SSID \"%s\" has no associated network interface. Will use the primary interface.\n", dssidName, pssidName);
      }
    }


    //ensure each primary SSID has an associated network interface
    auto sifnIter = _ssidIfnameMap.find(pssidName);
    if (sifnIter == _ssidIfnameMap.end()) {
      dcwlogerrf("Configured primary SSID \"%s\" has no associated network interface\n", pssidName);
      throw ValidationFailureException();
    }
  }

  //ensure we have at least one traffic filter progile
  if (_trafficFilterProfiles.empty()) {
    dcwlogerrf("%s\n", "We don't have a single traffic profile");
    throw ValidationFailureException();
  }

  //ensure we have a default traffic filter profile
  auto tfpIter = _trafficFilterProfiles.find(_defaultProfileName);
  if (tfpIter == _trafficFilterProfiles.end()) {
    dcwlogerrf("We don't have a default traffic profile (\"%s\")!\n", _defaultProfileName);
    throw ValidationFailureException();
  }

  //ensure that the filter files are parsable...
  for (const auto &tfp : _trafficFilterProfiles) {
    if (tfp.second == NULL) {
      dcwlogerrf("NULL traffic filter profile: %s\n", tfp.first.c_str());
      throw ValidationFailureException();
    }
    if (!MacRemapperDriver::ValidateFilter(*tfp.second)) {
      dcwlogerrf("Failed to parse filter \"%s\"\n", tfp.second->GetName());
      throw ValidationFailureException();
    }
  }
}

