/*
 * \file EcalBarrelMonitorWriteIntegrityClient.cpp
 *
 *  $Date: 2005/11/07 16:32:55 $
 *  $Revision: 1.6 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "CalibCalorimetry/EcalDBInterface/interface/EcalCondDBInterface.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunTag.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunIOV.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunConsistencyDat.h"

#include "TROOT.h"

#include <iostream>
#include <math.h>

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

#include <signal.h>

using namespace std;

MonitorUserInterface* mui;

EcalCondDBInterface* econn;

bool exit_now = false;

void ctrl_c_intr(int sig) {

  exit_now = true;
  cout << "Exiting ..." << endl;

  return;
}

void runc_analysis(MonitorElement** me01, MonitorElement** me02, MonitorElement** me03, MonitorElement** me04) {

  try {
    cout << "Making connection ... " << flush;
    econn = new EcalCondDBInterface("pccmsecdb.cern.ch",
                                    "ecalh4db",
                                    "test06",
                                    "oratest06");
    cout << "done." << endl;
  } catch (runtime_error &e) {
    cerr << e.what() << endl;
    return;
  }

  // The objects necessary to identify a dataset
  RunTag runtag;
  RunIOV runiov;

  Tm startTm;
  Tm endTm;

  // Set the beginning time
  startTm.setToCurrentGMTime();
//  startTm.setToString("2007-01-01 00:00:00");
  uint64_t microseconds = startTm.microsTime();

  run_t run = 14358;

  cout << "Setting run " << run << " start_time " << startTm.str() << endl;

  EcalLogicID ecid;
  RunConsistencyDat runc;
  map<EcalLogicID, RunConsistencyDat> dataset;

  // Set the properties of the tag
  runtag.setRunType("Laser");
  runtag.setLocation("H4");
  runtag.setMonitoringVersion("version 1");

  uint64_t oneMinute = 1 * 60 * 1000000;

  startTm.setToMicrosTime(microseconds);
  endTm.setToMicrosTime(microseconds + oneMinute);

  runiov.setRunNumber(run);
  runiov.setRunStart(startTm);
  runiov.setRunEnd(endTm);

  float n_min_tot = 1000.;
  float n_min_bin = 50.;

  TProfile2D* h01;
  TProfile2D* h02;
  TProfile2D* h03;
  TProfile2D* h04;

  cout << "Writing RunConsistencyDatObjects to database ..." << endl;

  MonitorElementT<TNamed>* ob;

  for ( int ism = 1; ism <= 36; ism++ ) {

    float num01, num02, num03, num04, num05;

    h01 = h02 = h03 = h04 = 0;

    if ( me01[ism-1] || me02[ism-1] ) {

      if ( me01[ism-1] ) {
        ob = dynamic_cast<MonitorElementT<TNamed>*> (me01[ism-1]);
        if ( ob ) {
          h01 = dynamic_cast<TProfile2D*> (ob->operator->());
        }
      }

      if ( me02[ism-1] ) {
        ob = dynamic_cast<MonitorElementT<TNamed>*> (me02[ism-1]);
        if ( ob ) {
          h02 = dynamic_cast<TProfile2D*> (ob->operator->());
        }
      }

    }

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        num01  = num02  = num03  = num04  = num05  = -1.;

        bool update_channel = false;

        if ( h01 && h01->GetEntries() >= n_min_tot ) {
          num01 = h01->GetBinEntries(h01->GetBin(ie, ip));
          update_channel = true;
        }

        if ( h02 && h02->GetEntries() >= n_min_tot ) {
          num02 = h02->GetBinEntries(h02->GetBin(ie, ip));
          update_channel = true;
        }

        if ( h03 && h03->GetEntries() >= n_min_tot ) {
          num03 = h03->GetBinEntries(h03->GetBin(ie, ip));
          update_channel = true;
        }

        if ( h04 && h04->GetEntries() >= n_min_tot ) {
          num04 = h04->GetBinEntries(h04->GetBin(ie, ip));
          update_channel = true;
        }

        if ( update_channel ) {

//          if ( ie == 1 && ip == 1 ) {

            cout << "Inserting dataset for SM=" << ism << endl;
            cout << "(" << ie << "," << ip << ") " << num01 << " " << num02 << " " << num03 << " " << num04 << " " << num05 << endl;

//          }

          runc.setExpectedEvents(num01);

          runc.setProblemsInId(num02);
          runc.setProblemsInSample(num03);
          runc.setProblemsInADC(num04);
          runc.setProblemsInGain(num05);

          try {
            ecid = econn->getEcalLogicID("EB_crystal_index", ism, ie-1, ip-1);
            dataset[ecid] = runc;
          } catch (runtime_error &e) {
            cerr << e.what() << endl;
          }

        }

      }
    }

  }

  try {
    cout << "Inserting dataset ... " << flush;
    econn->insertDataSet(&dataset, &runiov, &runtag );
    cout << "done." << endl;
  } catch (runtime_error &e) {
    cerr << e.what() << endl;
  }

  cout << "Closing connection." << endl;
  delete econn;

  return;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Barrel Write Integrity Monitor Client ***" << endl;
  cout << endl;

  signal(SIGINT, ctrl_c_intr);

  // default client name
  string cfuname = "UserWriteLaser";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  if ( argc >= 2 ) cfuname = argv[1];
  if ( argc >= 3 ) hostname = argv[2];

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  mui = new MonitorUIRoot(hostname, port_no, cfuname);

  mui->setVerbose(1);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("*/EcalBarrel/STATUS");
  mui->subscribe("*/EcalBarrel/RUN");
  mui->subscribe("*/EcalBarrel/EVT");
  mui->subscribe("*/EcalIntegrity/Gain/EI gain SM*");
  mui->subscribe("*/EcalIntegrity/ChId/EI ChId SM*");
  mui->subscribe("*/EcalIntegrity/TTId/EI TTId SM*");
  mui->subscribe("*/EcalIntegrity/TTBlockSize/EI TTBlockSize SM*");
  mui->subscribe("*/EcalIntegrity/DCC size error");

  int last_update = -1;
  int last_update2 = -1;

  bool stay_in_loop = true;

  while ( stay_in_loop && ! exit_now ) {

    // this is the "main" loop where we receive monitoring
    stay_in_loop = mui->update();

    // subscribe to new monitorable matching pattern
    mui->subscribeNew("*/EcalBarrel/STATUS");
    mui->subscribeNew("*/EcalBarrel/RUN"); 
    mui->subscribeNew("*/EcalBarrel/EVT");
    mui->subscribeNew("*/EcalIntegrity/Gain/EI gain SM*");
    mui->subscribeNew("*/EcalIntegrity/ChId/EI ChId SM*");
    mui->subscribeNew("*/EcalIntegrity/TTId/EI TTId SM*");
    mui->subscribeNew("*/EcalIntegrity/TTBlockSize/EI TTBlockSize SM*");
    mui->subscribeNew("*/EcalIntegrity/DCC size error");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    string s;
    string status;
    string run;
    string evt;

    MonitorElement* me01[36];
    MonitorElement* me02[36];
    MonitorElement* me03[36];
    MonitorElement* me04[36];

    bool update_db = false;

    // access monitoring objects every monitoring cycle
    if ( updates != last_update2 ) {

      me = mui->get("Collector/FU0/EcalBarrel/STATUS");
      if ( me ) {
        s = me->valueString();
        status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/RUN");
      if ( me ) {
        s = me->valueString();
        run = s.substr(2,s.length()-2);
        cout << "run = " << run << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/EVT");
      if ( me ) {
        s = me->valueString();
        evt = s.substr(2,s.length()-2);
        cout << "event = " << evt.c_str() << endl;
      }

      last_update2 = updates;
    }

    // access monitoring objects every 50 monitoring cycles, and
    // in any case at the end of run
    if ( ( updates % 50 == 0 && updates != last_update ) ||
         ( status == "end-of-run" ) ) {

      for ( int ism = 1; ism <= 36; ism++ ) me01[ism-1] = me02[ism-1] = 0;

      Char_t histo[150];

      for ( int ism = 1; ism <= 36; ism++ ) {

        sprintf(histo, "Collector/FU0/EcalIntegrity/ChId/EI ChId SM%02d", ism);
        me01[ism-1] = mui->get(histo);
        if ( me01[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

        sprintf(histo, "??", ism);
        me02[ism-1] = mui->get(histo);
        if ( me02[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

        sprintf(histo, "??", ism);
        me03[ism-1] = mui->get(histo);
        if ( me03[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

        sprintf(histo, "Collector/FU0/EcalIntegrity/Gain/EI gain SM%02d", ism);
        me04[ism-1] = mui->get(histo);
        if ( me04[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

      }

      last_update = updates;

      if ( update_db && status == "end-of-run" ) runc_analysis(me01, me02, me03, me04);
    }
  }

  mui->unsubscribe("*");

  usleep(100);

  delete mui;

  return 0;

}

