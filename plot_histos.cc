#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "GTKTypeDefs.hh"
#include "Packet32.hh"
#include "TDCPixTimeStampCollection.hh"
#include "TDCPixTimeStamp.hh"
#include "TH1F.h"
#include "TH2F.h"
#include "TBenchmark.h"
#include "TRandom.h"
#include "RConfig.h"
#include <stddef.h>
#include "TClass.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TError.h"
#include "TString.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TGFrame.h"
#include "TGTab.h"
#include "TGButton.h"
#include "TRootEmbeddedCanvas.h"
#include "TSystem.h"
#include "TRandom.h"
#include "TError.h"
	
using namespace std;
using namespace GTK;
int main(int argc, char **argv)
{
int option(0);
//unsigned int nFrames(0xffff);
unsigned int nFrameMax = -1;
  
/*
  while ((option = getopt(argc, argv,"n:i:")) != -1) {
    switch (option) {
      
    case 'n':{
      nFrameMax = atol(optarg);
      break;
    }
    }
  }
*/
  TFile* file=new TFile(argv[2], "RECREATE");
  // Constants
  const int wordLength = sizeof(u32);
  const int headerLength = 5;
  u32 headerBuf_1[headerLength];
  Packet32 p32_1;
  TDCPixTimeStampCollection tsColl_1;
  u32 headerBuf_2[headerLength];
  Packet32 p32_2;
  TDCPixTimeStampCollection tsColl_2;
  TH2F *histo1 = new TH2F("histo1"," Hit Map; X Pixel; Y Pixel",40,0,40,45,0,45);
  TH2F *histo6 = new TH2F("histo6"," Hit Map; X Pixel; Y Pixel",40,0,40,45,0,45);
  TH1F *histo3 = new TH1F("histo3"," Time Profile; Time [s]; Count",1000*60,0,10*60);
  TH1F *histo2 = new TH1F("histo2"," TOT; TOT [ns]; Count", 200, 0, 200);
  TH1F *histo4 = new TH1F("histo4","Time difference; t2-t1[ns]; Count", 500, -10, 10);
  double binw = TDCPixTimeStamp::ClockPeriod/256;
  TH2F *histo5 = new TH2F("histo5","Time difference vs. TOT;   TOT [ns]; Time difference t2-t1 [ns]", 500, 0, 2000*binw, 400, -binw*200, binw*200); 
  TH1F *histo7 = new TH1F("histo7","Time distribution; t[ns]; Count", 80, -binw*40,binw*40);
   
 
  // Read file to open from filelist
  ifstream dataStream1;
  ifstream dataStream2;
  ifstream listStream;
  string line[100];

  //read file list
  listStream.open (argv[1],  std::ios::in);
  int nFile(0);
  if (listStream.is_open())  {
    while ( getline (listStream,line[nFile]) )  nFile++; 
    listStream.close(); 
  }
  else {
    cerr << "Unable to open file"<<endl;
    exit(1);
  }

  //prepare to read files
  unsigned int iF(0);
  unsigned int tmax(0);
  unsigned int tmax2(0);
  dataStream1.open(line[0].c_str(), std::ios::in|std::ios::binary);
  dataStream2.open(line[1].c_str(), std::ios::in|std::ios::binary);

  dataStream1.seekg(0,dataStream1.end);
  double filesize = double(dataStream1.tellg());
  dataStream1.seekg(0,dataStream1.beg);

  while(dataStream1.good() && nFrameMax > 0 && iF<nFrameMax){

    if (iF%10000 == 0) cout<<" Read "<<100*double(dataStream1.tellg())/filesize<<"% or "<<100*iF/double(nFrameMax)<< "% \r"<<cout.flush();
    p32_1.Reset();
    tsColl_1.Clear();
    dataStream1.read ( (char*)&headerBuf_1[0],  headerLength*wordLength );
    for (int i(0);i<headerLength;i++) p32_1.SetHeaderWord(headerBuf_1[i],i);
    u32 payLoadLength_1 = headerBuf_1[4];
    u32 payLoadBuf_1[payLoadLength_1];
    dataStream1.read ( (char*)&payLoadBuf_1[0],  payLoadLength_1*wordLength);
    if( !dataStream1.good()){
      //dataStream1.close();
      break;
    }
    p32_1.SetPayloadPointer(payLoadBuf_1,payLoadLength_1);
    tsColl_1.Load(p32_1);
    tmax = tsColl_1.FrameStart().GetFrameCount();
      
    if( dataStream2.good()){
      while(tmax2<tmax){
	p32_2.Reset();
	tsColl_2.Clear();
	dataStream2.read ( (char*)&headerBuf_2[0],  headerLength*wordLength );
	for (int i(0);i<headerLength;i++) p32_2.SetHeaderWord(headerBuf_2[i],i);
	u32 payLoadLength_2 = headerBuf_2[4];
	u32 payLoadBuf_2[payLoadLength_2];
	dataStream2.read ( (char*)&payLoadBuf_2[0],  payLoadLength_2*wordLength);
	if( !dataStream2.good()){
	  //dataStream2.close();
	  break;
	}
	p32_2.SetPayloadPointer(payLoadBuf_2,payLoadLength_2);
	tsColl_2.Load(p32_2);
	tmax2 = tsColl_2.FrameStart().GetFrameCount();
      }
    }   
    
    
    for(unsigned int idx = 0; idx!=tsColl_1.Count(); idx++) {
      TDCPixTimeStamp hit = tsColl_1[idx];

      histo1->Fill(hit.GetNaturalColumnIndex(),hit.GetNaturalPixelIndex());	
      histo2->Fill(hit.GetTrailingTime()-hit.GetLeadingTime());
      histo3->Fill(hit.GetLeadingTime()*1e-9);
	
      for(unsigned int idx2 = 0; idx2!=tsColl_2.Count(); idx2++) {
	TDCPixTimeStamp hit2 = tsColl_2[idx2];
	if(hit.GetNaturalColumnIndex()==22 && hit.GetNaturalPixelIndex()==22 && abs(hit.GetLeadingTime()-hit2.GetLeadingTime())<5)
	  histo6->Fill(hit2.GetNaturalColumnIndex(),hit2.GetNaturalPixelIndex());
	
	if( hit.GetNaturalColumnIndex()==22 && hit.GetNaturalPixelIndex()==22 &&
	    abs(hit.GetLeadingTime()-hit2.GetLeadingTime()) < 5 &&
	    hit2.GetNaturalColumnIndex()==16 && hit2.GetNaturalPixelIndex()==20 &&
	    abs(hit.GetTrailingTime()-hit.GetLeadingTime() - 16.5)<0.5 && 
	    abs(hit2.GetTrailingTime()-hit2.GetLeadingTime() - 16.5)<0.5)
	  {
	    histo7->Fill(hit.GetLeadingTime()-hit2.GetLeadingTime());
	  }
	
	histo4->Fill(hit.GetLeadingTime()-hit2.GetLeadingTime());	  
	histo5->Fill(hit.GetTrailingTime()-hit.GetLeadingTime(),hit.GetLeadingTime()-hit2.GetLeadingTime());
	
      }
    } 
      
    iF++;   
  }
  histo1->Write();
  histo2->Write();
  histo3->Write();
  histo4->Write();
  histo5->Write();
  histo6->Write();
  histo7->Write();
  return 0;
} 

