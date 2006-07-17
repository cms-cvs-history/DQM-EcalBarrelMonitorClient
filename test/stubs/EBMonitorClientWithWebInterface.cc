
#include "EBMonitorClientWithWebInterface.h"

EBMonitorClientWithWebInterface::EBMonitorClientWithWebInterface(xdaq::ApplicationStub *stub) :
DQMBaseClient( stub,                                  // the application stub - do not change
               "EBMonitorClientWithWebInterface",     // the name by which the collector identifies the client
               "localhost",                           // the name of the computer hosting the collector
                9090,                                 // the port at which the collector listens
                5,                                    // the delay between reconnect attempts
                false                                 // do not act as server
             )
{

  bool webInterface = false;

  if ( webInterface ) {
    webInterface_p = new EBMonitorClientWebInterface(getContextURL(), getApplicationURL(), &mui_);
  }

  xgi::bind(this, &EBMonitorClientWithWebInterface::handleWebRequest, "Request");

  ebmc_ = 0;

}

void EBMonitorClientWithWebInterface::general(xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception)
{
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << "<head>" << std::endl;
  *out << "<META HTTP-EQUIV=refresh CONTENT=\"5\">";
  *out << "<title> " << "ecbm" << "</title>"  << std::endl;
  *out << "</head>" << std::endl;
  *out << "<body>" << std::endl;

  *out << cgicc::h3( "EcalBarrelMonitorClient Status" ).set( "style", "font-family:arial" ) << std::endl;

  if( ebmc_ ) {

    *out << "<table style=\"font-family: arial\"><tr><td>" << std::endl;

    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>" 
	 << "<tr><th>Cycle</th><td align=right>" << ebmc_->getEvtPerJob();
    int nevt = 0;
    if( ebmc_->getEntryHisto() != 0 ) nevt = int( ebmc_->getEntryHisto()->GetEntries());
    *out << "<tr><th>Event</th><td align=right>" << nevt
	 << "</td><tr><th>Run</th><td align=right>" << ebmc_->getRun() 
	 << "</td><tr><th>Run Type</th><td align=right> " << ebmc_->getRunType() 
	 << "</td></table></p>" << std::endl;

    *out << "</td><td>" << std::endl;

    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>" 
	 << "<tr><th>Evt Type</th><th>Evt/Run</th><th>Evt Type</th><th>Evt/Run</th>" << std::endl;
    vector<string> runTypes = ebmc_->getRunTypes();
    for( unsigned int i=0, j=0; i<runTypes.size(); i++ ) {
      if( runTypes[i] != "UNKNOWN" ) {
	if( j++%2 == 0 ) *out << "<tr>";
	nevt = 0;
	if( ebmc_->getEntryHisto() != 0 ) nevt = int( ebmc_->getEntryHisto()->GetBinContent(i+1));
	*out << "<td>" << runTypes[i] 
	     << "</td><td align=right>" << nevt << std::endl;
      }
    }
    *out << "</td></table></p>" << std::endl;

    *out << "</td><tr><td colspan=2>" << std::endl;

    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>" 
	 << "<tr><th>Client</th><th>Cyc/Job</th><th>Cyc/Run</th><th>Client</th><th>Cyc/Job</th><th>Cyc/Run</th>" << std::endl;
    const vector<EBClient*> clients = ebmc_->getClients();
    const vector<string> clientNames = ebmc_->getClientNames();
    for( unsigned int i=0; i<clients.size(); i++ ) {
      if( clients[i] != 0 ) {
	if( i%2 == 0 ) *out << "<tr>";
	*out << "<td>" << clientNames[i] 
	     << "</td><td align=right>" << clients[i]->getEvtPerJob()
	     << "</td><td align=right>" << clients[i]->getEvtPerRun() << std::endl;
      }
    }
    *out << "</td></table></p>" << std::endl;

    *out << "</td><tr><td>" << std::endl;


    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>"
	 << "<tr><th colspan=2>RunIOV</th>"
	 << "<tr><td>Run Number</td><td align=right> " << ebmc_->getRunIOV().getRunNumber()
	 << "</td><tr><td>Run Start</td><td align=right> " << ebmc_->getRunIOV().getRunStart().str()
	 << "</td><tr><td>Run End</td><td align=right> " << ebmc_->getRunIOV().getRunEnd().str()
	 << "</td></table></p>" << std::endl;

    *out << "</td><td colsapn=2>" << std::endl;

    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>" 
	 << "<tr><th colspan=2>RunTag</th>"
	 << "<tr><td>GeneralTag</td><td align=right> " << ebmc_->getRunIOV().getRunTag().getGeneralTag()
	 << "</td><tr><td>Location</td><td align=right> " << ebmc_->getRunIOV().getRunTag().getLocationDef().getLocation()
	 << "</td><tr><td>Run Type</td><td align=right> " << ebmc_->getRunIOV().getRunTag().getRunTypeDef().getRunType()
	 << "</td></table></p>" << std::endl;

    *out << "</td><tr><td>" << std::endl;

    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>" 
	 << "<tr><th colspan=2>MonRunIOV</th>"
	 << "<tr><td>SubRun Number</td><td align=right> " << ebmc_->getMonIOV().getSubRunNumber()
	 << "</td><tr><td>SubRun Start</td><td align=right> " << ebmc_->getMonIOV().getSubRunStart().str()
	 << "</td><tr><td>SubRun End</td><td align=right> " << ebmc_->getMonIOV().getSubRunEnd().str()
	 << "</td></table></p>" << std::endl;

    *out << "</td><td colspan=2>" << std::endl;

    *out << "<p style=\"font-family: arial\">" 
	 << "<table border=1>" 
	 << "<tr><th colspan=2>MonRunTag</th>"
	 << "<tr><td>GeneralTag</td><td align=right> " << ebmc_->getMonIOV().getMonRunTag().getGeneralTag()
	 << "</td><tr><td>Monitoring Version</td><td align=right> " << ebmc_->getMonIOV().getMonRunTag().getMonVersionDef().getMonitoringVersion()
	 << "</td></table></p>" << std::endl;

    *out << "</td><table>" << std::endl;

  }
  if ( webInterface_p ) webInterface_p->Default(in, out);
}

void EBMonitorClientWithWebInterface::handleWebRequest(xgi::Input *in, xgi::Output *out)
{

  if ( webInterface_p ) webInterface_p->handleRequest(in, out);

}

void EBMonitorClientWithWebInterface::configure()
{

  edm::ParameterSet ps;

  ps.addUntrackedParameter<string>("location", "H4");

  ps.addUntrackedParameter<bool>("collateSources", false);

  ps.addUntrackedParameter<bool>("cloneME", true);

  ps.addUntrackedParameter<bool>("enableQT", false);

  ps.addUntrackedParameter<bool>("enableSubRun", false);

  ps.addUntrackedParameter<bool>("enableExit", false);

  ps.addUntrackedParameter<bool>("enableMonitorDaemon", true);

  ps.addUntrackedParameter<string>("prefixME", "Collector/FU0/");
//  ps.addUntrackedParameter<string>("prefixME", "EvF/FU0/");

//DQMBaseClient: ps.addUntrackedParameter<bool>("enableServer", true);

  vector<int> superModules;
  superModules.push_back(1);

  ps.addUntrackedParameter<vector<int> >("superModules", superModules);

  ps.addUntrackedParameter<bool>("verbose", false);

  ebmc_ = new EcalBarrelMonitorClient(ps, mui_);

}

void EBMonitorClientWithWebInterface::newRun()
{

  upd_->registerObserver(this);

  if( ebmc_ ) ebmc_->beginJob();

}

void EBMonitorClientWithWebInterface::endRun()
{

  if( ebmc_ ) ebmc_->endJob();

}

void EBMonitorClientWithWebInterface::onUpdate() const
{

  if( ebmc_ ) ebmc_->analyze();

}

void EBMonitorClientWithWebInterface::finalize()
{

  if( ebmc_ ) delete ebmc_;

}

