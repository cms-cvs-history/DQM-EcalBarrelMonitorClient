/*
 * \file EBMonitorClientStateMachine.cc
 *
 * $Date: 2006/02/03 11:12:23 $
 * $Revision: 1.1 $
 * \author G. Della Ricca
 *
*/

#include "EBMonitorClientStateMachine.h"

class EBMonitorClientStateMachine : public DQMBaseClient, public dqm::UpdateObserver{

public:

XDAQ_INSTANTIATOR();

EBMonitorClientStateMachine(xdaq::ApplicationStub *s) : DQMBaseClient(s, "EBMonitorClientStateMachine"){ }

// called by configureAction()
void configure(){

  edm::ParameterSet ps;

  ps.addUntrackedParameter<string>("clientName", "test");

  ps.addUntrackedParameter<string>("hostName", "localhost");
  ps.addUntrackedParameter<int>("hostPort", 9090);

//  ps.addUntrackedParameter<string>("outputFile", "EcalBarrelMonitorClient.root");

//  ps.addUntrackedParameter<string>("dbName", "ecalh4db");
//  ps.addUntrackedParameter<string>("dbHostName", "pccmsecdb.cern.ch");
//  ps.addUntrackedParameter<string>("dbUserName", "test06");
//  ps.addUntrackedParameter<string>("dbPassword", "oratest06");
  ps.addUntrackedParameter<bool>("enableSubRun", true);

  ps.addUntrackedParameter<string>("location", "H4");

//  ps.addUntrackedParameter<string>("baseHtmlDir", ".");

//  ps.addUntrackedParameter<bool>("collateSources", true);
//  ps.addUntrackedParameter<bool>("cloneME", false);

  ps.addUntrackedParameter<bool>("enableExit", false);

//  ps.addUntrackedParameter<bool>("verbose", true);

  ebmc_ = new EcalBarrelMonitorClient(ps, mui_);

}

// called by enableAction()
void newRun(){

  upd_->registerObserver(this);

  ebmc_->beginJob();

//  ebmc_->beginRun();

}

// called by haltAction()
void endRun(){

  ebmc_->endRun();

}

// called by ~DQMBaseClient()
void finalize(){

  ebmc_->endJob();

  if ( ebmc_ ) delete ebmc_;

}

// called by Updater()
void onUpdate() const {

//  std::vector<std::string> uplist;
//  mui_->getUpdatedContents(uplist);
//  std::cout << "updated objects" << std::endl;
//  for(unsigned i=0; i<uplist.size(); i++)
//    std::cout << uplist[i] << std::endl;

  ebmc_->analyze();

}

private:

EcalBarrelMonitorClient* ebmc_;

};

XDAQ_INSTANTIATOR_IMPL(EBMonitorClientStateMachine)

