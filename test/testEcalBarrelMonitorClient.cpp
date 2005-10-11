/*
 * \file EcalBarrelMonitorClient.cpp
 *
 *  $Date: 2005/10/11 13:39:36 $
 *  $Revision: 1.2 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include <iostream>
#include <math.h>

#include "TROOT.h"
#include <TApplication.h>

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  cout << endl;
  cout << " *** Ecal Barrel Monitor Client ***" << endl;
  cout << endl;

  TApplication app("app",&argc,argv);

  // default client name
  string cfuname = "User0";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  TCanvas* c1 = new TCanvas("Ecal Barrel","Monitoring objects",200,10,600,480);
  c1->Draw();
  c1->Modified();

  if(argc >= 2) cfuname = argv[1];
  if(argc >= 3) hostname = argv[2];

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  MonitorUserInterface* mui= new MonitorUIRoot(hostname,port_no,cfuname);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("EcalBarrel/EBMonitorEvent/EB*");

  bool stay_in_loop = true;

  // last time monitoring objects were plotted
  int last_plotting = -1;

  // last time root-file was saved
  int last_save = -1;

  while ( stay_in_loop ) {

      bool saveHistograms = false;
  
      // this is the "main" loop where we receive monitoring
      stay_in_loop = mui->update();

      // subscribe to new monitorable matching pattern
      mui->subscribeNew("EcalBarrel/EBMonitorEvent/EB*SM01*");

      // # of full monitoring cycles processed
      int updates = mui->getNumUpdates();

      // draw all monitoring objects every 5 monitoring cycles
      if(updates % 5 == 0 && updates != last_plotting)
        {
//          mui->drawAll();

          MonitorElement * me = mui->get("Collector/FU0/EcalBarrel/EBMonitorEvent/EBMM event SM01");
          MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
          if ( ob ) {
            ob->operator->()->Draw();
            c1->Modified();
            c1->Update();
          }

          last_plotting = updates;
        }

      // come here every 100 monitoring cycles, operate on Monitoring Elements
      if(updates % 100 == 0 && updates != last_save)
        {
          saveHistograms = true;

          last_save = updates;
        }

      // save monitoring structure in root-file
      if(saveHistograms)
        mui->save("EcalBarrelMonitorClient.root");

    }

  // if here (ie. Collector has stopped), save into root file
  mui->save("EcalBarrelMonitorClient.root");

  delete mui;

  return 0;
}
