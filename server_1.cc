#include "TSocket.h"
#include "TCanvas.h"
#include "TSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TServerSocket.h"
#include "TGFrame.h"
#include "TGTab.h"
#include "TGButton.h"
#include "TRootEmbeddedCanvas.h"
#include "TGClient.h"
#include "TSystem.h"
#include "TH1.h"
#include "TRandom.h"
#include "TH2F.h"
#include "TError.h"
#include "TString.h"
#include <iostream>


using namespace std;

int server( int nClient, int nHisto) {
  
  
  if(nClient>10) {
    printf("Too many clients, max is 10\n");
    return 1;
  
  }

  //place for defining Canvas. tab  widgets and tabs ..............................................................................................
 
  gSystem->Load("libGui");
  
  TGMainFrame *main = new TGMainFrame(gClient->GetRoot(), 800, 600);
  TGTab *fTab = new TGTab(main, 300, 300);

  //  TGHorizontalFrame *frame = new TGHorizontalFrame(main, 60, 20, kFixedWidth);
  //  main->AddFrame(frame, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1));
    
  //--------- create the Tab widget
  const int nTab = nHisto;
  const int nPad = nClient;

  TGLayoutHints* fLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5);
  TGCompositeFrame *fTabCF[nTab];
  TGCompositeFrame *fCF[nTab];
  //TGLayoutHints *fL[nTab];
  TRootEmbeddedCanvas *fEC[nTab];
  TCanvas* cv[nTab];
  for (int i=0; i<nTab; i++) {
    fTabCF[i] = fTab->AddTab(Form("Histo %d",i+1));
    fCF[i] = new TGCompositeFrame(fTabCF[i],60,20,kHorizontalFrame);
    fEC[i] = new TRootEmbeddedCanvas(Form("ec%d",i+1), fTabCF[i], 100, 100);

    fCF[i]->AddFrame(fEC[i],fLayout);
    fTabCF[i]->AddFrame(fCF[i],fLayout);

    cv[i]=fEC[i]->GetCanvas();
    cv[i]->Divide(nPad,1);
  }
  main->AddFrame(fTab, new TGLayoutHints(kLHintsBottom|kLHintsExpandX|kLHintsExpandY,2,2,5,1));  
  main->MapSubwindows();
  main->Resize(main->GetDefaultSize());
  main->MapWindow();
  main->SetWindowName("Online Monitor");

     
  for (int i=0; i<nTab; i++) {
    cv[i]->Draw();
  }
  
  
  // Open a server socket looking for connections on a named service or
  // on a specified port.
  //TServerSocket *ss = new TServerSocket("rootserv", kTRUE);
  TServerSocket *ss = new TServerSocket(9090, kTRUE);
  TMonitor *mon = new TMonitor;
   
   
  TSocket* socks[10][10];
  for (int i(0);i < nClient; i++){
    for (int j(0); j<nHisto; j++) {
      socks[i][j] = ss->Accept();
      mon->Add(socks[i][j]);
    }
    socks[i][0]->Send(Form("%d",i));
  }

  //................................................
  while(mon->GetActive() != 0){
    TMessage *mess;
    TSocket  *s;
    s = mon->Select();
    s->Recv(mess);
	    
    if (mess->What() == kMESS_STRING) {
      char str[64];
      mess->ReadString(str, 64);
      printf("%s\n",str); 
      mon->Remove(s);
    }	 
    else if (mess->What() == kMESS_OBJECT) {
      TH1* histo = (TH1 *)mess->ReadObject(mess->GetClass());
      
      if (histo) {  
	int iPad(0); int iTab(0); bool found(0);
	for(int i=0; i<nClient; i++) {
	  for(int j=0; j<nHisto; j++){
	    if (s==socks[i][j]) { 
	      iPad = i;
	      iTab = j;
	    }
	  }
	}
	cv[iTab]->cd(iPad+1);
	gPad->Clear();
	histo->DrawCopy("COLZ"); 
	cv[iTab]->Modified();
	cv[iTab]->Update();
	delete histo;   
      }
    }

  }
 
 
	    
  // Close the socket.
  for (int i(0);i < nClient; i++){
    for(int k=0; k<nHisto; k++){
      socks[i][k]->Close();
    }
  }
  

}


int main(){
  int server();
  return 0;
}
