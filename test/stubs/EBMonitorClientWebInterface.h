#ifndef _EBMonitorClientWebInterface_h_
#define _EBMonitorClientWebInterface_h_

/*
  Ecal Barrel Monitor Client Web Interface
*/

#include "DQMServices/WebComponents/interface/WebInterface.h"

class EBMonitorClientWebInterface : public WebInterface
{

public:

  EBMonitorClientWebInterface(std::string theContextURL, std::string theApplicationURL, MonitorUserInterface ** _mui_p);

  /*
    you need to implement this function if you have widgets that invoke custom-made
    methods defined in your client
  */
  void handleCustomRequest(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  /*
    An example custom-made method that we want to bind to a widget
  */
  void CustomRequestResponse(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

private:

};

#endif // _EBMonitorClientWebInterface_h_
