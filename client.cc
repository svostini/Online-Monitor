#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "GTKTypeDefs.hh"
#include "Packet32.hh"
#include "TDCPixTimeStampCollection.hh"
#include "TDCPixTimeStamp.hh"
#include "TSocket.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMessage.h"
#include "TBenchmark.h"
#include "TRandom.h"
#include "RConfig.h"
#include <stddef.h>
#include "TClass.h"
#include <iostream>
#include "TCanvas.h"
#include "TPad.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TError.h"
#include "TString.h"
#include "TGraph.h"
using namespace std;
using namespace GTK;
int main(int argc, char **argv)
{
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
  TH1F *histo3 = new TH1F("histo3"," Time Profile; Time [s]; Count",1000*60,0,10*60);
  TH1F *histo2 = new TH1F("histo2"," TOT; TOT [ns]; Count", 200, 0, 200);
  TH1F *histo4 = new TH1F("histo4","Time difference; t2-t1[ns]; Count", 500, -10, 10);
  double binw = TDCPixTimeStamp::ClockPeriod/256;
  TH2F *histo5 = new TH2F("histo5","Time difference vs. TOT;   TOT [ns]; Time difference t2-t1 [ns]", 500, 0, 2000*binw, 400, -binw*200, binw*200); 

  double axMax = histo3->GetXaxis()->GetBinCenter(histo3->GetXaxis()->GetLast());
  double axMin = histo3->GetXaxis()->GetBinCenter(histo3->GetXaxis()->GetFirst());

  // Open connection to server (creating a socket).......................
  
  TSocket *socks[5];
  for (int i(0);i<5;i++){
    socks[i] = new TSocket("localhost", 9090);

  }
  // Wait till we get the start message
  char str[72] ="";
  socks[0]->Recv(str, 72);
  int clientID = atoi(str);
 
  // Read file to open from filelist
  ifstream dataStream1;
  ifstream dataStream2;
  ifstream listStream;
  string line[100];

  string prevFiles[100];
  for (int i(0);i<100;i++) prevFiles[i]="";
  int prevNFile(0);
  int fPosition1(0);
  int fPosition2(0);

  while(1){ // loop while waiting for new files

    //read file list
    listStream.open ("/afs/cern.ch/work/s/svostini/TDCPixDecoder/data/fileList.txt",  std::ios::in);
    int nFile(0);
    if (listStream.is_open())  {
      while ( getline (listStream,line[nFile]) )  nFile++; 
      listStream.close(); 
    }
    else {
      cerr << "Unable to open file"<<endl;
      exit(1);
    }

    //check that we have a new file
    bool newFile(1);
    for(int i(0);i<prevNFile;i++){
      for(int j(0);j<nFile;j++){
	cout<<prevFiles[i].c_str()<<" vs "<<line[j].c_str()<<"\r"<<cout.flush();
	if( strcmp(prevFiles[i].c_str(),line[j].c_str()) == 0 )  newFile = 0;
      }
    }
    cout<<"new file: "<<newFile<<"\r"<<cout.flush();
    if( newFile == 0 )  sleep(2);


    //store the file names
    for(int i(0);i<nFile;i++)  prevFiles[i] = line[i];
    prevNFile = nFile;

				
    //open the data file if new files
    if( newFile == 1 ){
      printf("%s\n",line[clientID].c_str()); 
      dataStream1.open(line[clientID].c_str(), std::ios::in|std::ios::binary);

      if(!dataStream1.is_open()) {
	cerr<<"file "<<line[clientID].c_str()<<"could not be opened"<<endl;
	exit(1); 
      } 
    
      dataStream2.open(line[(clientID+1)%nFile].c_str(), std::ios::in|std::ios::binary);
      if(!dataStream2.is_open()) {
	cerr<<"file "<<line[clientID].c_str()<<"could not be opened"<<endl;
	exit(1); 
      } 
    }

    //prepare to read files
    int iF(0);
    histo1->Reset();  
    histo2->Reset();  
    histo3->Reset();  
    histo4->Reset();
    histo5->Reset();
    histo6->Reset();
    histo7->Reset();
    TMessage mess(kMESS_OBJECT);
    unsigned int tmax(0);
    unsigned int tmax2(0);

    //reset the steam in case we are reading the same file but 
    //it was just updated
    dataStream1.clear();
    dataStream2.clear();
    dataStream1.seekg(fPosition1,dataStream1.beg);
    dataStream2.seekg(fPosition2,dataStream2.beg);

    while(dataStream1.good()){

      fPosition1 = dataStream1.tellg();
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
	  fPosition2 = dataStream1.tellg();
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
	//    	cout<<" "<<idx<<"  x: "<<hit.GetNaturalColumnIndex()<<" y "<<hit.GetNaturalPixelIndex()<<endl;
	histo1->Fill(hit.GetNaturalColumnIndex(),hit.GetNaturalPixelIndex());
	histo2->Fill(hit.GetTrailingTime()-hit.GetLeadingTime());
	histo3->Fill(hit.GetLeadingTime()*1e-9);
	for(unsigned int idx2 = 0; idx2!=tsColl_2.Count(); idx2++) {
	  TDCPixTimeStamp hit2 = tsColl_2[idx2];
 
	  histo4->Fill(hit.GetLeadingTime()-hit2.GetLeadingTime());
	  histo5->Fill(hit.GetTrailingTime()-hit.GetLeadingTime(),hit.GetLeadingTime()-hit2.GetLeadingTime());
	}
      } 
      
      iF++;

        if(iF%10000000 ==0){
	  histo1->Reset();
	  cout<<iF<<"\r"<<cout.flush();
     
        }
        if(iF%100000 ==0){
	mess.Reset();              // re-use TMessage object
	mess.WriteObject(histo1);     // write object in message buffer
	socks[0]->Send(mess);          // send message

	mess.Reset();
	mess.WriteObject(histo2);     // write object in message buffer
	socks[1]->Send(mess);          // send message

	mess.Reset();
	if(tmax*6.4e-6 < axMin + 60)   histo3->GetXaxis()->SetRangeUser(axMin, axMin+60);
        else   histo3->GetXaxis()->SetRangeUser(tmax*6.4e-6 - 60, tmax*6.4e-6);
       
	mess.WriteObject(histo3);     // write object in message buffer
	socks[2]->Send(mess);          // send message

	if(tmax*6.4e-6 > axMax) {
	  cout<<"tmax "<<tmax<< " aMax "<<axMax<<endl;
	  histo3->Delete();
	  histo3 = new TH1F("histo3"," Time Profile; Time [s]; Count",1000*60,axMax,axMax+60*10);
	  axMax = histo3->GetXaxis()->GetBinCenter(histo3->GetXaxis()->GetLast());
	  axMin = histo3->GetXaxis()->GetBinCenter(histo3->GetXaxis()->GetFirst());

	}
        mess.Reset();
	mess.WriteObject(histo4);   //write object in message buffer
	socks[3]->Send(mess);       //send message

	
	mess.Reset();
	mess.WriteObject(histo5);   //write object in message buffer
	socks[4]->Send(mess);       //send message

        
      }
    }
  }
  socks[0]->Send(Form("This is the last message from client %d",clientID));
  socks[1]->Send(Form("This is the last message from client %d",clientID));
  socks[2]->Send(Form("This is the last message from client %d",clientID));
  socks[3]->Send(Form("This is the last message from client %d",clientID));
  socks[4]->Send(Form("This is the last message from client %d",clientID));
  return 0;
} 

