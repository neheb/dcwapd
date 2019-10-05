
#include "./filters.hpp"


#include "dcw/filetrafficfilterprofile.hpp"
#include "dcwposix/filterdirscanner.hpp"
#include "dcwlinux/macremapper_driver.hpp"
#include "ccspwrapper/tr181_config_provider.h"

namespace {

//two for one...
//this works perfect for this because both consumers
//(tr181 and dcw config) delete when they are done
struct SingleFilter :
  public ::dcw::FileTrafficFilterProfile,
  public ::ccspwrapper::Tr181ConfigProvider {

  SingleFilter(const ::dcw::FileTrafficFilterProfile& rhv) :
    ::dcw::FileTrafficFilterProfile(rhv) {}
  virtual ~SingleFilter() = default;
  virtual void GetValue(const char * const name, ::ccspwrapper::Tr181Scalar& value) {
    //theres only one value... the name...
    value = this->GetName();
  }
};

} // namespace


Filters::Filters() {
  _scanPath = "/var/run/dcw/filters";
}

Filters::~Filters() {
  //
}

const char *Filters::GetScanPath() const {
  return _scanPath.c_str();
}

void Filters::SetScanPath(const char * const scanPath) {
  _scanPath = scanPath;
}


void Filters::PopulateConfigProviderCollection(::ccspwrapper::Tr181ConfigProvider& parent, ConfigProviderCollection& collection) {
  //
  ::dcwlinux::APConfigurationProvider::CFTFPList filters;
  InstanciateCFileTrafficFilterProfiles(filters);
  for (const auto &filter : filters) {
    collection.push_back((SingleFilter*)filter);
  }
}

void Filters::InstanciateCFileTrafficFilterProfiles(::dcwlinux::APConfigurationProvider::CFTFPList& output) const {
  try {
    ::dcwposix::FilterdirScanner::FileFilterProfileList ffpl;
    ::dcwposix::FilterdirScanner dirScanner(_scanPath.c_str());
    dirScanner.Scan(ffpl);
    for(const auto &ffp : ffpl) {
      if (!::dcwlinux::MacRemapperDriver::ValidateFilter(ffp)) continue;

      //note: caller (either dcw config or ccsp/tr181) will cleanup
      output.push_back(new SingleFilter(ffp));
    }
  }
  catch (...) {
    //probably should care more if this fails...
  }
}

