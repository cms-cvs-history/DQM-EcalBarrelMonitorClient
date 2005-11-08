/*
 * \file EcalBarrelMonitorWritePedestalClient.cpp
 *
 *  $Date: 2005/11/06 14:22:58 $
 *  $Revision: 1.7 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "CalibCalorimetry/EcalDBInterface/interface/EcalCondDBInterface.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunTag.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunIOV.h"
#include "CalibCalorimetry/EcalDBInterface/interface/MonPedestalsDat.h"

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

void pedestals_analysis(MonitorElement** me01, MonitorElement** me06, MonitorElement** me12) {

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

  run_t run = 14316;

  cout << "Setting run " << run << " start_time " << startTm.str() << endl;

  EcalLogicID ecid;
  MonPedestalsDat ped;
  map<EcalLogicID, MonPedestalsDat> dataset;

  // Set the properties of the tag
  runtag.setRunType("Pedestals");
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
  TProfile2D* h06;
  TProfile2D* h12;

  cout << "Writing MonPedestalsDatObjects to database ..." << endl;

  MonitorElementT<TNamed>* ob;

  for ( int ism = 1; ism <= 36; ism++ ) {

    float num01, num06, num12;
    float mean01, mean06, mean12;
    float rms01, rms06, rms12;

    h01 = h06 = h12 = 0;

    if ( me01[ism-1] || me06[ism-1] || me12[ism-1] ) {

      if ( me01[ism-1] ) {
        ob = dynamic_cast<MonitorElementT<TNamed>*> (me01[ism-1]);
        if ( ob ) {
          h01 = dynamic_cast<TProfile2D*> (ob->operator->());
        }
      }

      if ( me06[ism-1] ) {
        ob = dynamic_cast<MonitorElementT<TNamed>*> (me06[ism-1]);
        if ( ob ) {
          h06 = dynamic_cast<TProfile2D*> (ob->operator->());
        }
      }

      if ( me12[ism-1] ) {
        ob = dynamic_cast<MonitorElementT<TNamed>*> (me12[ism-1]);
        if ( ob ) {
          h12 = dynamic_cast<TProfile2D*> (ob->operator->());
        }
      }

    }

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        num01  = num06  = num12  = -1.;
        mean01 = mean06 = mean12 = -1.;
        rms01  = rms06  = rms12  = -1.;

        bool update_channel = false;

        if ( h01 && h01->GetEntries() >= n_min_tot ) {
          num01 = h01->GetBinEntries(h01->GetBin(ie, ip));
          if ( num01 >= n_min_bin ) {
            mean01 = h01->GetBinContent(h01->GetBin(ie, ip));
            rms01  = h01->GetBinError(h01->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( h06 && h06->GetEntries() >= n_min_tot ) {
          num06 = h06->GetBinEntries(h06->GetBin(ie, ip));
          if ( num06 >= n_min_bin ) {
            mean06 = h06->GetBinContent(h06->GetBin(ie, ip));
            rms06  = h06->GetBinError(h06->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( h12 && h12->GetEntries() >= n_min_tot ) {
          num12 = h12->GetBinEntries(h12->GetBin(ie, ip));
          if ( num12 >= n_min_bin ) {
            mean12 = h12->GetBinContent(h12->GetBin(ie, ip));
            rms12  = h12->GetBinError(h12->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( update_channel ) {

//          if ( ie == 1 && ip == 1 ) {

            cout << "Inserting dataset for SM=" << ism << endl;
            cout << "G01 (" << ie << "," << ip << ") " << num01 << " " << mean01 << " " << rms01 << endl;
            cout << "G06 (" << ie << "," << ip << ") " << num06 << " " << mean06 << " " << rms06 << endl;
            cout << "G12 (" << ie << "," << ip << ") " << num12 << " " << mean12 << " " << rms12 << endl;

//          }

          ped.setPedMeanG1(mean01);
          ped.setPedRMSG1(rms01);

          ped.setPedMeanG6(mean06);
          ped.setPedRMSG6(rms06);

          ped.setPedMeanG12(mean12);
          ped.setPedRMSG12(rms12);

          ped.setTaskStatus(1);

          try {
            ecid = econn->getEcalLogicID("EB_crystal_index", ism, ie-1, ip-1);
            dataset[ecid] = ped;
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
  cout << " *** Ecal Barrel Write Pedestal Monitor Client ***" << endl;
  cout << endl;

  signal(SIGINT, ctrl_c_intr);

  // default client name
  string cfuname = "UserWritePedestal";

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
  mui->subscribe("*/EcalBarrel/RUNTYPE");
  mui->subscribe("*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM*");
  mui->subscribe("*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM*");
  mui->subscribe("*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM*");

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
    mui->subscribeNew("*/EcalBarrel/RUNTYPE");
    mui->subscribeNew("*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM*");
    mui->subscribeNew("*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM*");
    mui->subscribeNew("*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM*");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    string s;
    string status;
    string run;
    string evt;
    string type;

    MonitorElement* me01[36];
    MonitorElement* me06[36];
    MonitorElement* me12[36];

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
        cout << "event = " << evt << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/RUNTYPE");
      if ( me ) {
        s = me->valueString();
        type = s.substr(2,s.length()-2);
        cout << "type = " << type << endl;
      }

      last_update2 = updates;
    }

    // access monitoring objects every 50 monitoring cycles, and
    // in any case at the end of run
    if ( ( type == "2" ) && 
         ( ( updates % 50 == 0 && updates != last_update ) || ( status == "end-of-run" )    ) ) {

      for ( int ism = 1; ism <= 36; ism++ ) me01[ism-1] = me06[ism-1] = me12[ism-1] = 0;

      Char_t histo[150];

      for ( int ism = 1; ism <= 36; ism++ ) {

        sprintf(histo, "Collector/FU0/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
        me01[ism-1] = mui->get(histo);
        if ( me01[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

        sprintf(histo, "Collector/FU0/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
        me06[ism-1] = mui->get(histo);
        if ( me06[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }

        sprintf(histo, "Collector/FU0/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
        me12[ism-1] = mui->get(histo);
        if ( me12[ism-1] ) {
          cout << "Found '" << histo << "'" << endl;
          update_db = true;
        }
      }

      last_update = updates;

      if ( update_db && status == "end-of-run" ) pedestals_analysis(me01, me06, me12);
    }
  }

  mui->unsubscribe("*");

  usleep(100);

  delete mui;

  return 0;

}

