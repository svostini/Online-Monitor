#ifndef __Packet32_HH__
#define __Packet32_HH__

#include <stdint.h>
#include "GTKTypeDefs.hh"


#include <iostream>

namespace GTK{

  class Packet32{


  public: 
  
  
    Packet32(u32 buflen=8192);
    virtual ~Packet32(); 
  
  
    void Reset();
  


    /*
     * The packet needs to be able to tell us a few 
     * things about itself: I'd like this to be neater, 
     * but expediency is more important at the mo.
     * MN.
     */
    
    int IsTimeStampPacket()const; 
    int TimeStampCount()const; 
    int QChip()const; 


    u32 DataTag0()const; 
    u32 DataTag1()const; 






    u16 GetHeaderLength()const;

    // u16 GetLogicalPipeID()const;
    // void SetLogicalPipeID(u16 pipeid);

    int SetHeaderWord(u32 word, u32 offset); 
    int GetHeaderWord(u32 & word, u32 offset)const; 
    
    
    //  int_first<=
    //       card_id    &  ---> physical card id
    //       spare_id   &  ---> chip id
    //       routing_id &  ---> destination routing info
    //       channel_id ;  ---> sub-channel id //quarter chip?
    
    
    void SetReadoutCardID(u16 rocardid);
    u16 GetReadoutCardID()const;
  
    u16 GetChipID()const; 
    void SetChipID(u16 chipid); 
  
    u16 GetRoutingID()const;
    void SetRoutingID(u16 routeid);
    
    u16 GetChannelID()const;
    void SetChannelID(u16 channelid);

    u16 GetGTK()const;
    void SetGTK(u16 gtk);

    
    void SetPayloadLength(u32 aPayloadLength);
    u32 GetPayloadLength()const;
    
    u32 GetPacketLength()const;
    const u32 * GetBuffer()const;
    u32 * GetBuffer();

    u32 GetBurstNumber()const; //added by mpt
    
    const u32 * GetPayload()const;
    u32 * GetPayload();
    
    void SetPayloadPointer( u32 * aPayload, 
		    u32 aLength);

    void SetPayload(const u32 * aPayload, 
		    u32 aLength);

    void AppendToPayload(const u32 * aPayloadAddition, 
			 u32 aLength);
        
    u32 GetPayload(u32 * aBuf, 
		   u32 aBufLen)const;
        
    
    u32 GetInternalBufferLength()const; 


    int operator==(const Packet32 & rhs)const; 
    int operator!=(const Packet32 & rhs)const; 
        
  
  
    friend std::ostream & operator<<(std::ostream & aOs,
				     const Packet32 & pkt);





   
    

    // returns true if CRCs match
    // false if not.
    bool CheckCRC()const;
    
    void AddCRC();
    
    u32 GetCRC()const;
    u32 CalculateCRC()const;

  private: 
    
    u32 UpdateCRC(u32 aData, u32 aCRC)const;
    

  private: 
  
    u32 mHeaderLength;
    u32 * mBuf; 
    u32 mBufLen; 
    u32 * mPayload;
    u32 * mPayloadLength; 
    u16 mGtk;
  };
  
  
  std::ostream & operator<<(std::ostream & aOs, 
			    const Packet32 & pkt);
  
  
  



}


#endif


