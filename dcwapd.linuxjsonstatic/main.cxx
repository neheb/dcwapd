
#include <cstring>

#include "dcwposix/processsignalmanager.hpp"
#include "dcwposix/eventreactorexitsignaler.hpp"
#include "dcwposix/selecteventreactor.hpp"
#include "dcwposix/filterdirscanner.hpp"

#include "dcwlinux/macremapper_driver.hpp"
#include "dcwlinux/ap_configuration.hpp"
#include "dcwlinux/vap_manager.hpp"
#include "dcwlinux/json_configuration_provider.hpp"

#include "dcw/dcwlog.hpp"

#include <exception>

int
main( void ) {

  try {
    dcwlinux::JsonConfigurationProvider      configProvider("./dcwapdconf.json");

    dcwposix::ProcessSignalManager           sigman;
    dcwposix::SelectEventReactor             eventReactor;
    dcwposix::EventReactorExitSignalHandler  eventReactorCleanExit(sigman, eventReactor);

    dcwlinux::MacRemapperDriver              driver;
    dcwlinux::APConfiguration                conf(configProvider);
    dcwlinux::VAPManager                     vapman;

    conf.Apply(driver, vapman, eventReactor);

    driver.Dump();

    eventReactor.Run();
  }
  catch (std::exception& e) {
    dcwlogerrf("Caught main() exception: %s\n", e.what());
    return 1;
  }
  return 0;
}

