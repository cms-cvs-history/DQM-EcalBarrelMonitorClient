#include "EBMonitorClientWithWebInterface.h"


EBMonitorClientWithWebInterface::EBMonitorClientWithWebInterface(xdaq::ApplicationStub *stub) 
  : DQMBaseClient(
		  stub,       // the application stub - do not change
		  "test",     // the name by which the collector identifies the client
		  "localhost",// the name of the computer hosting the collector
		  9090        // the port at which the collector listens
		  )
{
  // Instantiate a web interface:
  webInterface_p = new EBMonitorClientWebInterface(getContextURL(),getApplicationURL(), & mui_);
  
  xgi::bind(this, &EBMonitorClientWithWebInterface::handleWebRequest, "Request");
}

/*
  implement the method that outputs the page with the widgets (declared in DQMBaseClient):
*/
void EBMonitorClientWithWebInterface::general(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  // the web interface should know what to do:
  webInterface_p->Default(in, out);
}


/*
  the method called on all HTTP requests of the form ".../Request?RequestID=..."
*/
void EBMonitorClientWithWebInterface::handleWebRequest(xgi::Input * in, xgi::Output * out)
{
  // the web interface should know what to do:
  webInterface_p->handleRequest(in, out);
}

/*
  this obligatory method is called whenever the client enters the "Configured" state:
*/
void EBMonitorClientWithWebInterface::configure()
{

  edm::ParameterSet ps;

  ps.addUntrackedParameter<bool>("enableSubRun", true);
  ps.addUntrackedParameter<string>("location", "H4");
  ps.addUntrackedParameter<bool>("enableExit", false);
  ps.addUntrackedParameter<bool>("verbose", true);

  ebmc_ = new EcalBarrelMonitorClient(ps, mui_);
}

/*
  this obligatory method is called whenever the client enters the "Enabled" state:
*/
void EBMonitorClientWithWebInterface::newRun()
{
  upd_->registerObserver(this);

  ebmc_->beginJob();

}

/*
  this obligatory method is called whenever the client enters the "Halted" state:
*/
void EBMonitorClientWithWebInterface::endRun()
{
  ebmc_->endRun();
}

// called by ~DQMBaseClient()
void EBMonitorClientWithWebInterface::finalize(){

  ebmc_->endJob();

  if ( ebmc_ ) delete ebmc_;

}


/*
  this obligatory method is called by the Updater component, whenever there is an update 
*/
void EBMonitorClientWithWebInterface::onUpdate() const
{
  // put here the code that needs to be executed on every update:

  ebmc_->analyze();

  std::vector<std::string> uplist;
  mui_->getUpdatedContents(uplist);

}

