
#include "EBMonitorClientWithWebInterface.h"

EBMonitorClientWithWebInterface::EBMonitorClientWithWebInterface(xdaq::ApplicationStub *stub) :
DQMBaseClient( stub,                                  // the application stub - do not change
               "EBMonitorClientWithWebInterface",     // the name by which the collector identifies the client
               "localhost",                           // the name of the computer hosting the collector
                9090                                   // the port at which the collector listens
             )
{

  webInterface_p = new EBMonitorClientWebInterface(getContextURL(), getApplicationURL(), &mui_);
  
  xgi::bind(this, &EBMonitorClientWithWebInterface::handleWebRequest, "Request");

}

void EBMonitorClientWithWebInterface::general(xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception)
{

  webInterface_p->Default(in, out);
}

void EBMonitorClientWithWebInterface::handleWebRequest(xgi::Input *in, xgi::Output *out)
{

  webInterface_p->handleRequest(in, out);

}

void EBMonitorClientWithWebInterface::configure()
{

  edm::ParameterSet ps;

  ps.addUntrackedParameter<bool>("enableSubRun", true);
  ps.addUntrackedParameter<string>("location", "H4");

  ps.addUntrackedParameter<bool>("collateSources", false);

  ps.addUntrackedParameter<bool>("cloneME", false);

  ps.addUntrackedParameter<bool>("enableExit", false);

  ps.addUntrackedParameter<bool>("enableMonitorDaemon", true);

  ps.addUntrackedParameter<bool>("verbose", false);

  ebmc_ = new EcalBarrelMonitorClient(ps, mui_);

}

void EBMonitorClientWithWebInterface::newRun()
{

  upd_->registerObserver(this);

  ebmc_->beginJob();

}

void EBMonitorClientWithWebInterface::endRun()
{

  ebmc_->endJob();

}

void EBMonitorClientWithWebInterface::onUpdate() const
{

  ebmc_->analyze();

}

void EBMonitorClientWithWebInterface::finalize()
{

  delete ebmc_;

}

