#include "Packet32.hh"

#include "RG_Utilities.hh"

#include <string.h>


// eg141006: Packet32 is used by DataDeviceFileReader to receive calibration data from the TDCPix.
// 			 After reception Packet32 are then given as argument to the TDCPixTimeStampCollector for checking...


namespace GTK{


// acr 2014-04-28 letta e commentata; mBuf, mPayload, mPayloadLength sono tutti puntatori a u32 (array); mBufLen e mHeaderLength are u32
// acr 2014-04-28 mBuf punta all'inizio del buffer e il puntatore al payload e' mBuf+mHeaderLength = mBuf+5 per poter saltare le 5 parole u32 dell'header.
// acr 2014-04-28 evidentemente il puntatore alla mPayloadLength is mBuf+4 because the payload length is stored in the last u32 word of the header
	Packet32::Packet32(u32 buflen):
    mHeaderLength(5),
    mBuf(0),
    mBufLen(0), 
    mPayload(0), 
    mPayloadLength(0)
  {
  
    mBuf=new u32[buflen];
    if(mBuf){
      // correct allocation: 
      mBufLen=buflen; 
      memset(mBuf,  0, mBufLen*sizeof(u32)); 
      mPayload=(mBuf+mHeaderLength);			// acr 2014-04-28 mBuf punta all'inizio del buffer e il puntatore al payload e' mBuf+mHeaderLength = mBuf+5 per poter saltare le 5 parole u32 dell'header.
      mPayloadLength=(mBuf+mHeaderLength-1);	// acr 2014-04-28 evidentemente il puntatore alla mPayloadLength is mBuf+4 because the payload length is stored in the last u32 word of the header
    }else{
      RG_REPORT_ERROR_MSG("Error allocating memory buffer."); 
    }
  }


  Packet32::~Packet32()
  {
    delete [] mBuf; mBuf=0; 
    mBufLen=0; 
  
  }
  
  


  void Packet32::Reset(){ 
    memset(mBuf,  0, mBufLen*sizeof(u32)); 
  }


  

  
  int Packet32::IsTimeStampPacket()const
  {
    /* data packets come from channel 0: */
    if(GetChannelID()==0){return 1;}
    return 0; 
  }
  
  int Packet32::TimeStampCount()const{
    /* each time stamp is 48 bits wrapped
     * on 1.5 words (6 bytes). The frame
     * words are always present as is 
     * the CRC.
     */
    u32 payload_len=GetPayloadLength();
    if(payload_len==0){return 0;}
    
    int nts=static_cast<int>(((payload_len-1)*4)/6); // *num bytes per u32 /num bytes per TS
    nts-=2;// subtract the frame words: 
    if(mPayload[payload_len-2]==0xffffffff){/* need to see if there are padding words*/
      nts--; 
    }
    
    return nts; 
  }

  int Packet32::QChip()const
  {
    u32 h1=0;
    GetHeaderWord(h1, 1);
    return static_cast<int>(h1&0x3); 
  }
  


  u32 Packet32::DataTag0()const
  {
    u32 h2=0;
    GetHeaderWord(h2, 2);
    
    return h2 & 0x0fffffff; 
  }
  u32 Packet32::DataTag1()const 
  {
    u32 h3=0;
    GetHeaderWord(h3, 3);
    return h3; 
  }

  u32 Packet32::GetBurstNumber()const
  {
    return DataTag0();
  }














  u16 Packet32::GetHeaderLength()const{ 
    return mHeaderLength; 
  }
  
  
  int Packet32::SetHeaderWord(u32 word, u32 offset)
  {
    RG_CHECK_INDEX_UPPER_BOUND_RETURN(offset, this->GetHeaderLength()-1);
    mBuf[offset]=word;
    return 0; 
  }

  int Packet32::GetHeaderWord(u32 & word, u32 offset)const{
    RG_CHECK_INDEX_UPPER_BOUND_RETURN(offset, this->GetHeaderLength()-1);
    word=mBuf[offset];
    return 0; 
  }




  // int_first<=card_id & spare_id & routing_id & channel_id ; 

  // the spare has been allocated to the chip_id in this firmware build.
  

  void Packet32::SetReadoutCardID(u16 rocardid){ 
    // we have an 8 bit space in the 
    // first word, so mask out anything else. 
    if(rocardid & 0xff00){
      RG_REPORT_WARNING_MSG("bits will be truncated.");
    }
    
    u32 temp=rocardid & 0xff; 
    temp<<=24;
    mBuf[0]=(mBuf[0]&0xffffff)|temp; 
  }

  u16 Packet32::GetReadoutCardID()const{
    u16 temp=(mBuf[0]>>24)&0xff; 
    return temp; 
  }
  
  void Packet32::SetGTK(u16 gtk){ 
    mGtk = gtk;
  }

  u16 Packet32::GetGTK() const {
    return mGtk;
  }

  void Packet32::SetChipID(u16 chipid){ 
    if(chipid & 0xff00){
      RG_REPORT_WARNING_MSG("bits will be truncated.");
    }
    u32 temp=chipid&0xff; 
    temp<<16;
    mBuf[0]=(mBuf[0]&0xff00ffff)|temp; 
  } 
  
  
  u16 Packet32::GetChipID()const{ 
    u16 temp=(mBuf[0]>>16)&0xff; 
    return temp; 
  }
  
  void Packet32::SetRoutingID(u16 routeid){ 
    if(routeid & 0xff00){
      RG_REPORT_WARNING_MSG("bits will be truncated.");
    }
    u32 temp=routeid<<8; 
    mBuf[0]=(mBuf[0]&0xffff00ff)|temp;     
  } 
  

  u16 Packet32::GetRoutingID()const{ 
    u16 temp=(mBuf[0]>>8)&0xff; 
    return temp; 
  } 
  
  void Packet32::SetChannelID(u16 channelid){ 
    if(channelid & 0xff00){
      RG_REPORT_WARNING_MSG("bits will be truncated.");
    }
    u32 temp=channelid; 
    mBuf[0]=(mBuf[0]&0xffffff00)|temp; 
  } 
  
  u16 Packet32::GetChannelID()const{ 
    u16 temp=(mBuf[0]&0xff);
    return temp;
  } 
  

  
    
    








  
  void Packet32::SetPayloadLength(u32 aPayloadLength){ 
    (*mPayloadLength)=aPayloadLength;
    //mBuf[mHeaderLength-1]=mPayloadLength;
  }



  u32 Packet32::GetPayloadLength()const{ 
    
    return (*mPayloadLength); 
  }
  

  u32 Packet32::GetPacketLength()const
  {
    return (this->GetPayloadLength()+
	    this->GetHeaderLength());
  }
  
  const u32 * Packet32::GetBuffer()const
  {
    return mBuf; 
  }

  u32 * Packet32::GetBuffer()
  {
    return mBuf;
  }
  
  const u32 * Packet32::GetPayload()const
  {
    return mPayload; 
  }
   
  u32 * Packet32::GetPayload()
  {
    return mPayload; // acr 2014-04-28 Packet32.hh(130):    u32 * mPayload;
  }
    


  
  void Packet32::SetPayload(const u32 * aPayload, 
				   u32 aLength)
  {
    
    memcpy(mPayload, aPayload, aLength*sizeof(u32)); 
    this->SetPayloadLength(aLength); 
    //    this->AddCRC();
  }


  void Packet32::SetPayloadPointer( u32 * aPayload, 
				   u32 aLength)
  {

    mPayload = aPayload;
    this->SetPayloadLength(aLength); 
  }

  
  void Packet32::AppendToPayload(const u32 * aPayloadAddition, 
				 u32 aLength)
  {
    u32 length=GetPayloadLength(); 
    
    memcpy(reinterpret_cast<void *>(&mPayload[length-1]), 
	   aPayloadAddition, 
	   aLength*sizeof(u32)); 
    *mPayloadLength=length+aLength-1;
    this->AddCRC(); 
    
  }

    
  u32 Packet32::GetPayload(u32 * aBuf, 
			   u32 aBufLen)const
  {
    if((*mPayloadLength)>aBufLen){// not ok
      RG_REPORT_ERROR_MSG("Buffer passed has insufficient space for the payload.");
      return -1; 
    }
    if((*mPayloadLength)==0){
      RG_REPORT_WARNING_MSG("Zero space passed. This is probably not what you wanted to do.");
      return 0; 
    }

    // ok
    memcpy(aBuf, mPayload, ((*mPayloadLength)-1)*sizeof(u32));
    
    return (*mPayloadLength)-1; 
  }  
  

  u32 Packet32::GetInternalBufferLength()const{
    return mBufLen; 
  }


  int Packet32::operator==(const Packet32 & rhs)const
  {
    if(this==(&rhs)){
      // self comparison: return 1
      return 1; 
    }
    u32 thislen=this->GetPacketLength(); 

    int rv=memcmp(this->mBuf, rhs.mBuf, thislen*sizeof(u32)); 
    return rv?0:1;
  }



  int Packet32::operator!=(const Packet32 & rhs)const{
    return !this->operator==(rhs);
  }



  





  



  // returns true if CRCs match
  // false if not.
  bool Packet32::CheckCRC()const{
    return GetCRC()==CalculateCRC();    
  }
  
  void Packet32::AddCRC(){ // acr 2014-04-28
    (*mPayloadLength)++;    
    u32 crc=CalculateCRC();
    mBuf[mHeaderLength+(*mPayloadLength)-1]=crc; //acr 2014-04-28 Packet32.cc(12):    mHeaderLength(5),
  }
    
  u32 Packet32::GetCRC()const{
    return mBuf[GetPacketLength()-1];    
  }
  
  u32 Packet32::CalculateCRC()const{
    
    u32 crc=0xffffffff;
    u32 length=GetPacketLength()-1;
    for(u32 i(0); i!=length; i++){
      crc=UpdateCRC(mBuf[i], crc);
    }
    return crc;
  }
  
#define __BIT32(__BIT_NO, __WORD) (((__WORD)>>(__BIT_NO))&0x00000001)
  
  u32 Packet32::UpdateCRC(u32 data, u32 crc)const{
    u32 newcrc=0;
    
    // this implementation was taken directly from the VHDL via a small sed script. 
    
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(30,data) ^ __BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(16,data) ^ __BIT32(12,data) ^ __BIT32(10,data) ^ __BIT32( 9,data) ^ __BIT32( 6,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 6, crc) ^ __BIT32( 9, crc) ^ __BIT32(10, crc) ^ __BIT32(12, crc) ^ __BIT32(16, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc) ^ __BIT32(30, crc) ^ __BIT32(31, crc)) & 1) << 0 ;
    newcrc |= ( (__BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(24,data) ^ __BIT32(17,data) ^ __BIT32(16,data) ^ __BIT32(13,data) ^ __BIT32(12,data) ^ __BIT32(11,data) ^ __BIT32( 9,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32( 9, crc) ^ __BIT32(11, crc) ^ __BIT32(12, crc) ^ __BIT32(13, crc) ^ __BIT32(16, crc) ^ __BIT32(17, crc) ^ __BIT32(24, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc)) & 1) << 1 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(30,data) ^ __BIT32(26,data) ^ __BIT32(24,data) ^ __BIT32(18,data) ^ __BIT32(17,data) ^ __BIT32(16,data) ^ __BIT32(14,data) ^ __BIT32(13,data) ^ __BIT32( 9,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32( 9, crc) ^ __BIT32(13, crc) ^ __BIT32(14, crc) ^ __BIT32(16, crc) ^ __BIT32(17, crc) ^ __BIT32(18, crc) ^ __BIT32(24, crc) ^ __BIT32(26, crc) ^ __BIT32(30, crc) ^ __BIT32(31, crc)) & 1) << 2 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(27,data) ^ __BIT32(25,data) ^ __BIT32(19,data) ^ __BIT32(18,data) ^ __BIT32(17,data) ^ __BIT32(15,data) ^ __BIT32(14,data) ^ __BIT32(10,data) ^ __BIT32( 9,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32( 9, crc) ^ __BIT32(10, crc) ^ __BIT32(14, crc) ^ __BIT32(15, crc) ^ __BIT32(17, crc) ^ __BIT32(18, crc) ^ __BIT32(19, crc) ^ __BIT32(25, crc) ^ __BIT32(27, crc) ^ __BIT32(31, crc)) & 1) << 3 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(30,data) ^ __BIT32(29,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(20,data) ^ __BIT32(19,data) ^ __BIT32(18,data) ^ __BIT32(15,data) ^ __BIT32(12,data) ^ __BIT32(11,data) ^ __BIT32( 8,data) ^ __BIT32( 6,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 6, crc) ^ __BIT32( 8, crc) ^ __BIT32(11, crc) ^ __BIT32(12, crc) ^ __BIT32(15, crc) ^ __BIT32(18, crc) ^ __BIT32(19, crc) ^ __BIT32(20, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(29, crc) ^ __BIT32(30, crc) ^ __BIT32(31, crc)) & 1) << 4 ;
    newcrc |= ( (__BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(24,data) ^ __BIT32(21,data) ^ __BIT32(20,data) ^ __BIT32(19,data) ^ __BIT32(13,data) ^ __BIT32(10,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32(10, crc) ^ __BIT32(13, crc) ^ __BIT32(19, crc) ^ __BIT32(20, crc) ^ __BIT32(21, crc) ^ __BIT32(24, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc)) & 1) << 5 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(29,data) ^ __BIT32(25,data) ^ __BIT32(22,data) ^ __BIT32(21,data) ^ __BIT32(20,data) ^ __BIT32(14,data) ^ __BIT32(11,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32(11, crc) ^ __BIT32(14, crc) ^ __BIT32(20, crc) ^ __BIT32(21, crc) ^ __BIT32(22, crc) ^ __BIT32(25, crc) ^ __BIT32(29, crc) ^ __BIT32(30, crc)) & 1) << 6 ;
    newcrc |= ( (__BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(22,data) ^ __BIT32(21,data) ^ __BIT32(16,data) ^ __BIT32(15,data) ^ __BIT32(10,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 5,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 5, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32(10, crc) ^ __BIT32(15, crc) ^ __BIT32(16, crc) ^ __BIT32(21, crc) ^ __BIT32(22, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc)) & 1) << 7 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(28,data) ^ __BIT32(23,data) ^ __BIT32(22,data) ^ __BIT32(17,data) ^ __BIT32(12,data) ^ __BIT32(11,data) ^ __BIT32(10,data) ^ __BIT32( 8,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 8, crc) ^ __BIT32(10, crc) ^ __BIT32(11, crc) ^ __BIT32(12, crc) ^ __BIT32(17, crc) ^ __BIT32(22, crc) ^ __BIT32(23, crc) ^ __BIT32(28, crc) ^ __BIT32(31, crc)) & 1) << 8 ;
    newcrc |= ( (__BIT32(29,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(18,data) ^ __BIT32(13,data) ^ __BIT32(12,data) ^ __BIT32(11,data) ^ __BIT32( 9,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 9, crc) ^ __BIT32(11, crc) ^ __BIT32(12, crc) ^ __BIT32(13, crc) ^ __BIT32(18, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(29, crc)) & 1) << 9 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(26,data) ^ __BIT32(19,data) ^ __BIT32(16,data) ^ __BIT32(14,data) ^ __BIT32(13,data) ^ __BIT32( 9,data) ^ __BIT32( 5,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 5, crc) ^ __BIT32( 9, crc) ^ __BIT32(13, crc) ^ __BIT32(14, crc) ^ __BIT32(16, crc) ^ __BIT32(19, crc) ^ __BIT32(26, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc) ^ __BIT32(31, crc)) & 1) << 10 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(20,data) ^ __BIT32(17,data) ^ __BIT32(16,data) ^ __BIT32(15,data) ^ __BIT32(14,data) ^ __BIT32(12,data) ^ __BIT32( 9,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 9, crc) ^ __BIT32(12, crc) ^ __BIT32(14, crc) ^ __BIT32(15, crc) ^ __BIT32(16, crc) ^ __BIT32(17, crc) ^ __BIT32(20, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc) ^ __BIT32(31, crc)) & 1) << 11 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(30,data) ^ __BIT32(27,data) ^ __BIT32(24,data) ^ __BIT32(21,data) ^ __BIT32(18,data) ^ __BIT32(17,data) ^ __BIT32(15,data) ^ __BIT32(13,data) ^ __BIT32(12,data) ^ __BIT32( 9,data) ^ __BIT32( 6,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 6, crc) ^ __BIT32( 9, crc) ^ __BIT32(12, crc) ^ __BIT32(13, crc) ^ __BIT32(15, crc) ^ __BIT32(17, crc) ^ __BIT32(18, crc) ^ __BIT32(21, crc) ^ __BIT32(24, crc) ^ __BIT32(27, crc) ^ __BIT32(30, crc) ^ __BIT32(31, crc)) & 1) << 12 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(28,data) ^ __BIT32(25,data) ^ __BIT32(22,data) ^ __BIT32(19,data) ^ __BIT32(18,data) ^ __BIT32(16,data) ^ __BIT32(14,data) ^ __BIT32(13,data) ^ __BIT32(10,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 5,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 5, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32(10, crc) ^ __BIT32(13, crc) ^ __BIT32(14, crc) ^ __BIT32(16, crc) ^ __BIT32(18, crc) ^ __BIT32(19, crc) ^ __BIT32(22, crc) ^ __BIT32(25, crc) ^ __BIT32(28, crc) ^ __BIT32(31, crc)) & 1) << 13 ;
    newcrc |= ( (__BIT32(29,data) ^ __BIT32(26,data) ^ __BIT32(23,data) ^ __BIT32(20,data) ^ __BIT32(19,data) ^ __BIT32(17,data) ^ __BIT32(15,data) ^ __BIT32(14,data) ^ __BIT32(11,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32(11, crc) ^ __BIT32(14, crc) ^ __BIT32(15, crc) ^ __BIT32(17, crc) ^ __BIT32(19, crc) ^ __BIT32(20, crc) ^ __BIT32(23, crc) ^ __BIT32(26, crc) ^ __BIT32(29, crc)) & 1) << 14 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(27,data) ^ __BIT32(24,data) ^ __BIT32(21,data) ^ __BIT32(20,data) ^ __BIT32(18,data) ^ __BIT32(16,data) ^ __BIT32(15,data) ^ __BIT32(12,data) ^ __BIT32( 9,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32( 9, crc) ^ __BIT32(12, crc) ^ __BIT32(15, crc) ^ __BIT32(16, crc) ^ __BIT32(18, crc) ^ __BIT32(20, crc) ^ __BIT32(21, crc) ^ __BIT32(24, crc) ^ __BIT32(27, crc) ^ __BIT32(30, crc)) & 1) << 15 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(29,data) ^ __BIT32(26,data) ^ __BIT32(24,data) ^ __BIT32(22,data) ^ __BIT32(21,data) ^ __BIT32(19,data) ^ __BIT32(17,data) ^ __BIT32(13,data) ^ __BIT32(12,data) ^ __BIT32( 8,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 8, crc) ^ __BIT32(12, crc) ^ __BIT32(13, crc) ^ __BIT32(17, crc) ^ __BIT32(19, crc) ^ __BIT32(21, crc) ^ __BIT32(22, crc) ^ __BIT32(24, crc) ^ __BIT32(26, crc) ^ __BIT32(29, crc) ^ __BIT32(30, crc)) & 1) << 16 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(30,data) ^ __BIT32(27,data) ^ __BIT32(25,data) ^ __BIT32(23,data) ^ __BIT32(22,data) ^ __BIT32(20,data) ^ __BIT32(18,data) ^ __BIT32(14,data) ^ __BIT32(13,data) ^ __BIT32( 9,data) ^ __BIT32( 6,data) ^ __BIT32( 5,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 5, crc) ^ __BIT32( 6, crc) ^ __BIT32( 9, crc) ^ __BIT32(13, crc) ^ __BIT32(14, crc) ^ __BIT32(18, crc) ^ __BIT32(20, crc) ^ __BIT32(22, crc) ^ __BIT32(23, crc) ^ __BIT32(25, crc) ^ __BIT32(27, crc) ^ __BIT32(30, crc) ^ __BIT32(31, crc)) & 1) << 17 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(28,data) ^ __BIT32(26,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(21,data) ^ __BIT32(19,data) ^ __BIT32(15,data) ^ __BIT32(14,data) ^ __BIT32(10,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 2,data) ^ __BIT32( 2, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32(10, crc) ^ __BIT32(14, crc) ^ __BIT32(15, crc) ^ __BIT32(19, crc) ^ __BIT32(21, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(26, crc) ^ __BIT32(28, crc) ^ __BIT32(31, crc)) & 1) << 18 ;
    newcrc |= ( (__BIT32(29,data) ^ __BIT32(27,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(22,data) ^ __BIT32(20,data) ^ __BIT32(16,data) ^ __BIT32(15,data) ^ __BIT32(11,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 3,data) ^ __BIT32( 3, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32(11, crc) ^ __BIT32(15, crc) ^ __BIT32(16, crc) ^ __BIT32(20, crc) ^ __BIT32(22, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(27, crc) ^ __BIT32(29, crc)) & 1) << 19 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(28,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(23,data) ^ __BIT32(21,data) ^ __BIT32(17,data) ^ __BIT32(16,data) ^ __BIT32(12,data) ^ __BIT32( 9,data) ^ __BIT32( 8,data) ^ __BIT32( 4,data) ^ __BIT32( 4, crc) ^ __BIT32( 8, crc) ^ __BIT32( 9, crc) ^ __BIT32(12, crc) ^ __BIT32(16, crc) ^ __BIT32(17, crc) ^ __BIT32(21, crc) ^ __BIT32(23, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(28, crc) ^ __BIT32(30, crc)) & 1) << 20 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(29,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(24,data) ^ __BIT32(22,data) ^ __BIT32(18,data) ^ __BIT32(17,data) ^ __BIT32(13,data) ^ __BIT32(10,data) ^ __BIT32( 9,data) ^ __BIT32( 5,data) ^ __BIT32( 5, crc) ^ __BIT32( 9, crc) ^ __BIT32(10, crc) ^ __BIT32(13, crc) ^ __BIT32(17, crc) ^ __BIT32(18, crc) ^ __BIT32(22, crc) ^ __BIT32(24, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(29, crc) ^ __BIT32(31, crc)) & 1) << 21 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(29,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(19,data) ^ __BIT32(18,data) ^ __BIT32(16,data) ^ __BIT32(14,data) ^ __BIT32(12,data) ^ __BIT32(11,data) ^ __BIT32( 9,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 9, crc) ^ __BIT32(11, crc) ^ __BIT32(12, crc) ^ __BIT32(14, crc) ^ __BIT32(16, crc) ^ __BIT32(18, crc) ^ __BIT32(19, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(29, crc) ^ __BIT32(31, crc)) & 1) << 22 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(29,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(20,data) ^ __BIT32(19,data) ^ __BIT32(17,data) ^ __BIT32(16,data) ^ __BIT32(15,data) ^ __BIT32(13,data) ^ __BIT32( 9,data) ^ __BIT32( 6,data) ^ __BIT32( 1,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 1, crc) ^ __BIT32( 6, crc) ^ __BIT32( 9, crc) ^ __BIT32(13, crc) ^ __BIT32(15, crc) ^ __BIT32(16, crc) ^ __BIT32(17, crc) ^ __BIT32(19, crc) ^ __BIT32(20, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(29, crc) ^ __BIT32(31, crc)) & 1) << 23 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(21,data) ^ __BIT32(20,data) ^ __BIT32(18,data) ^ __BIT32(17,data) ^ __BIT32(16,data) ^ __BIT32(14,data) ^ __BIT32(10,data) ^ __BIT32( 7,data) ^ __BIT32( 2,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 2, crc) ^ __BIT32( 7, crc) ^ __BIT32(10, crc) ^ __BIT32(14, crc) ^ __BIT32(16, crc) ^ __BIT32(17, crc) ^ __BIT32(18, crc) ^ __BIT32(20, crc) ^ __BIT32(21, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc) ^ __BIT32(30, crc)) & 1) << 24 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(22,data) ^ __BIT32(21,data) ^ __BIT32(19,data) ^ __BIT32(18,data) ^ __BIT32(17,data) ^ __BIT32(15,data) ^ __BIT32(11,data) ^ __BIT32( 8,data) ^ __BIT32( 3,data) ^ __BIT32( 2,data) ^ __BIT32( 2, crc) ^ __BIT32( 3, crc) ^ __BIT32( 8, crc) ^ __BIT32(11, crc) ^ __BIT32(15, crc) ^ __BIT32(17, crc) ^ __BIT32(18, crc) ^ __BIT32(19, crc) ^ __BIT32(21, crc) ^ __BIT32(22, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc) ^ __BIT32(31, crc)) & 1) << 25 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(28,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(22,data) ^ __BIT32(20,data) ^ __BIT32(19,data) ^ __BIT32(18,data) ^ __BIT32(10,data) ^ __BIT32( 6,data) ^ __BIT32( 4,data) ^ __BIT32( 3,data) ^ __BIT32( 0,data) ^ __BIT32( 0, crc) ^ __BIT32( 3, crc) ^ __BIT32( 4, crc) ^ __BIT32( 6, crc) ^ __BIT32(10, crc) ^ __BIT32(18, crc) ^ __BIT32(19, crc) ^ __BIT32(20, crc) ^ __BIT32(22, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(28, crc) ^ __BIT32(31, crc)) & 1) << 26 ;
    newcrc |= ( (__BIT32(29,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(21,data) ^ __BIT32(20,data) ^ __BIT32(19,data) ^ __BIT32(11,data) ^ __BIT32( 7,data) ^ __BIT32( 5,data) ^ __BIT32( 4,data) ^ __BIT32( 1,data) ^ __BIT32( 1, crc) ^ __BIT32( 4, crc) ^ __BIT32( 5, crc) ^ __BIT32( 7, crc) ^ __BIT32(11, crc) ^ __BIT32(19, crc) ^ __BIT32(20, crc) ^ __BIT32(21, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(29, crc)) & 1) << 27 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(22,data) ^ __BIT32(21,data) ^ __BIT32(20,data) ^ __BIT32(12,data) ^ __BIT32( 8,data) ^ __BIT32( 6,data) ^ __BIT32( 5,data) ^ __BIT32( 2,data) ^ __BIT32( 2, crc) ^ __BIT32( 5, crc) ^ __BIT32( 6, crc) ^ __BIT32( 8, crc) ^ __BIT32(12, crc) ^ __BIT32(20, crc) ^ __BIT32(21, crc) ^ __BIT32(22, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc) ^ __BIT32(30, crc)) & 1) << 28 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(25,data) ^ __BIT32(23,data) ^ __BIT32(22,data) ^ __BIT32(21,data) ^ __BIT32(13,data) ^ __BIT32( 9,data) ^ __BIT32( 7,data) ^ __BIT32( 6,data) ^ __BIT32( 3,data) ^ __BIT32( 3, crc) ^ __BIT32( 6, crc) ^ __BIT32( 7, crc) ^ __BIT32( 9, crc) ^ __BIT32(13, crc) ^ __BIT32(21, crc) ^ __BIT32(22, crc) ^ __BIT32(23, crc) ^ __BIT32(25, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc) ^ __BIT32(31, crc)) & 1) << 29 ;
    newcrc |= ( (__BIT32(30,data) ^ __BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(26,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(22,data) ^ __BIT32(14,data) ^ __BIT32(10,data) ^ __BIT32( 8,data) ^ __BIT32( 7,data) ^ __BIT32( 4,data) ^ __BIT32( 4, crc) ^ __BIT32( 7, crc) ^ __BIT32( 8, crc) ^ __BIT32(10, crc) ^ __BIT32(14, crc) ^ __BIT32(22, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(26, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc) ^ __BIT32(30, crc)) & 1) << 30 ;
    newcrc |= ( (__BIT32(31,data) ^ __BIT32(30,data) ^ __BIT32(29,data) ^ __BIT32(28,data) ^ __BIT32(27,data) ^ __BIT32(25,data) ^ __BIT32(24,data) ^ __BIT32(23,data) ^ __BIT32(15,data) ^ __BIT32(11,data) ^ __BIT32( 9,data) ^ __BIT32( 8,data) ^ __BIT32( 5,data) ^ __BIT32( 5, crc) ^ __BIT32( 8, crc) ^ __BIT32( 9, crc) ^ __BIT32(11, crc) ^ __BIT32(15, crc) ^ __BIT32(23, crc) ^ __BIT32(24, crc) ^ __BIT32(25, crc) ^ __BIT32(27, crc) ^ __BIT32(28, crc) ^ __BIT32(29, crc) ^ __BIT32(30, crc) ^ __BIT32(31, crc)) & 1) << 31 ;
    
    return newcrc;    
  }

 
  // friend function to print the contents in a human friendly format:

  std::ostream & operator<<(std::ostream & aOs, 
			    const Packet32 & pkt)
  {
  
    aOs<<"Payload length = "<<std::dec<<(*(pkt.mPayloadLength))<<'\n';
    u32 lTotLen=pkt.GetPacketLength();
    /*
    for(u32 i=0; i<(lTotLen); i++){
      if(i<pkt.mHeaderLength){
	aOs<<"Header  # "<<std::dec<<i<<" = ";
	aOs.fill('0');aOs.width(8);
	aOs<<std::hex<<pkt.mBuf[i]<<'\n';
      }
      else{
	aOs<<"Payload # "<<std::dec<<(i-pkt.mHeaderLength)<<" = ";
	aOs.fill('0');aOs.width(8);
	aOs<<std::hex<<pkt.mBuf[i]<<'\n';
	
    } }*/
    aOs<<" CRC = "<<std::hex<<pkt.mBuf[(pkt.mHeaderLength+(*pkt.mPayloadLength)-1)]<<std::endl;


  
    return aOs;
}

}
