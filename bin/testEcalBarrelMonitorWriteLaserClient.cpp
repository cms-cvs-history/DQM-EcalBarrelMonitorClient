/*
 * \file EcalBarrelMonitorLaserClient.cpp
 *
 *  $Date: 2005/11/06 14:22:58 $
 *  $Revision: 1.1 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "CalibCalorimetry/EcalDBInterface/interface/EcalCondDBInterface.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunTag.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunIOV.h"
#include "CalibCalorimetry/EcalDBInterface/interface/MonLaserRedDat.h"

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

void apd_analysis(MonitorElement** me01, MonitorElement** me02) {

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

  run_t run = 14317;

  cout << "Setting run " << run << " start_time " << startTm.str() << endl;

  EcalLogicID ecid;
  MonLaserRedDat apd;
  map<EcalLogicID, MonLaserRedDat> dataset;

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

  cout << "Writing MonLaserRedDatObjects to database ..." << endl;

  MonitorElementT<TNamed>* ob;

  for ( int ism = 1; ism <= 36; ism++ ) {

    float num01, num02;
    float mean01, mean02;
    float rms01, rms02;

    h01 = h02 = 0;

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

        num01  = num02  = -1.;
        mean01 = mean02 = -1.;
        rms01  = rms02  = -1.;

        bool update_channel = false;

        if ( h01 && h01->GetEntries() >= n_min_tot ) {
          num01 = h01->GetBinEntries(h01->GetBin(ie, ip));
          if ( num01 >= n_min_bin ) {
            mean01 = h01->GetBinContent(h01->GetBin(ie, ip));
            rms01  = h01->GetBinError(h01->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( h02 && h02->GetEntries() >= n_min_tot ) {
          num02 = h02->GetBinEntries(h02->GetBin(ie, ip));
          if ( num02 >= n_min_bin ) {
            mean02 = h02->GetBinContent(h02->GetBin(ie, ip));
            rms02  = h02->GetBinError(h02->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( update_channel ) {

//          if ( ie == 1 && ip == 1 ) {

            cout << "Inserting dataset for SM=" << ism << endl;
            cout << "L1 (" << ie << "," << ip << ") " << num01 << " " << mean01 << " " << rms01 << endl;
            cout << "L6 (" << ie << "," << ip << ") " << num02 << " " << mean02 << " " << rms02 << endl;

//          }

          apd.setAPDMean(mean01);
          apd.setAPDRMS(rms01);

          apd.setAPDOverPNMean(mean02);
          apd.setAPDOverPNRMS(rms02);

          apd.setTaskStatus(1);

          try {
            ecid = econn->getEcalLogicID("EB_crystal_index", ism, ie-1, ip-1);
            dataset[ecid] = apd;
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
  cout << " *** Ecal Barrel Write Laser Monitor Client ***" << endl;
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
  mui->subscribe("*/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude SM*");
  mui->subscribe("*/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude over PN SM*");

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
    mui->subscribeNew("*/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude SM*");
    mui->subscribeNew("*/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude over PN SM*");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    string s;
    string status;
    string run;
    string evt;

    MonitorElement* me01[36];
    MonitorElement* me02[36];

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

        sprintf(histo, "Collector/FU0/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude SM%02d L1", ism);
        me01[ism-1] = mui->get(histo);
        if ( me01[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

        sprintf(histo, "Collector/FU0/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude over PN SM%02d L1", ism);
        me02[ism-1] = mui->get(histo);
        if ( me02[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

      }

      last_update = updates;

      if ( update_db && status == "end-of-run" ) apd_analysis(me01, me02);
    }
  }

  mui->unsubscribe("*");

  usleep(100);

  delete mui;

  return 0;

}

