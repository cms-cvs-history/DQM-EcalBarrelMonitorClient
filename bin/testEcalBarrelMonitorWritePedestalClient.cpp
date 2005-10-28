/*
 * \file EcalBarrelMonitorPedestalClient.cpp
 *
 *  $Date: 2005/10/28 10:22:18 $
 *  $Revision: 1.2 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "TROOT.h"

#include <iostream>
#include <math.h>

#include <signal.h>

using namespace std;

MonitorUserInterface* mui;

bool exit_now = false;

void ctr_c_intr(int sig) {

  exit_now = true;
  cout << "Exiting ..." << endl;
  signal(SIGINT, ctr_c_intr);

  return;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Barrel Pedestal Monitor Client ***" << endl;
  cout << endl;

  signal(SIGINT, ctr_c_intr);

  // default client name
  string cfuname = "UserPedestal";

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
  mui->subscribe("*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM*");
  mui->subscribe("*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM*");
  mui->subscribe("*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM*");

  bool stay_in_loop = true;

  while ( stay_in_loop && ! exit_now ) {

    // this is the "main" loop where we receive monitoring
    stay_in_loop = mui->update();

    // subscribe to new monitorable matching pattern
    mui->subscribeNew("*/EcalBarrel/STATUS");
    mui->subscribeNew("*/EcalBarrel/RUN"); 
    mui->subscribeNew("*/EcalBarrel/EVT");
    mui->subscribeNew("*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM*");
    mui->subscribeNew("*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM*");
    mui->subscribeNew("*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM*");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    // draw monitoring objects every 5 monitoring cycles
    if ( updates % 5 == 0 ) {

      me = mui->get("Collector/FU0/EcalBarrel/STATUS");
      if ( me ) {
        string s = me->valueString();
        string status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/RUN");
      if ( me ) {
        string s = me->valueString();
        string run = s.substr(2,s.length()-2);
        cout << "run = " << run << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/EVT");
      if ( me ) {
        string s = me->valueString();
        string evt = s.substr(2,s.length()-2);
        cout << "event = " << evt.c_str() << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM01 G01");
      if ( me ) {
        MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
        if ( ob ) {
          TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
          if ( h ) {
            cout << "Found 'Gain01/EBPT pedestal SM01 G01'" << endl;
          }
        }
      }

      me = mui->get("Collector/FU0/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM01 G06");
      if ( me ) {
        MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
        if ( ob ) {
          TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
          if ( h ) {
            cout << "Found 'Gain06/EBPT pedestal SM01 G06'" << endl;
          }
        }
      }

      me = mui->get("Collector/FU0/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM01 G12");
      if ( me ) {
        MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
        if ( ob ) {
          TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
          if ( h ) {
            cout << "Found 'Gain12/EBPT pedestal SM01 G12'" << endl;
          }
        }
      }

    }
  }

  mui->unsubscribe("*");

  usleep(100);

  delete mui;

  return 0;

}

