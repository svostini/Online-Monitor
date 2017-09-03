#ifndef __TDCPixTimeStamp__HH__
#define __TDCPixTimeStamp__HH__

#define TDCPIX_TS_BUFFER_SIZE 6


#include <iostream>
#include <vector>


namespace GTK{

  int ConvertUID( unsigned int uid,
		  /* outputs */
		  unsigned int & chip,
		  unsigned int & column,
		  unsigned int & pixel);
    
  int ConvertMatrixIndices(unsigned int column, /* 0-39 */
			   unsigned int pixel,  /* 0-44 */
			   /* outputs */
			   unsigned int & qchip, 
			   unsigned int & pixel_group_address, 
			   unsigned int & hit_arbiter_address);
		       


  int ConvertMatrixIndices(unsigned int column_pair, /* 0-19 */
			   unsigned int odd_col,     /* 0-1  */
			   unsigned int pixel,       /* 0-44 */
			   /* outputs */
			   unsigned int & qchip, 
			   unsigned int & pixel_group_address, 
			   unsigned int & hit_arbiter_address);
		       

  int ConvertTimeStampIndices(unsigned int qchip, 
			      unsigned int pixel_group_address,
			      unsigned int hit_arbiter_address, 
			      /* outputs */
			      unsigned int & column, 
			      unsigned int & pixel); 

  int ConvertTimeStampIndices(unsigned int qchip, 
			      unsigned int pixel_group_address,
			      unsigned int hit_arbiter_address, 
			      /* outputs */
			      unsigned int & column_pair,
			      unsigned int & odd_col, 
			      unsigned int & pixel); 



  //class Packet32 ; 
  

  




  class TDCPixTimeStamp {

  public: 
  
    TDCPixTimeStamp(int serial_fragment=0); 
    ~TDCPixTimeStamp(); 
    static const double ClockPeriod;
    int Clear(); 

    /*  */
    double ComputeLeadingTime(const double& offset = 0, const bool& fCorr = 1); 
    double GetLeadingTime(bool = 1)const; 
    double GetTrailingTime(bool = 1)const; 


    unsigned short IsTimeStamp()const; 
    unsigned short IsPaddingWord()const; 

    /* time stamp data accessors */
  
    unsigned short GetTrailingFineTime()const; 
    unsigned short GetTrailingCoarseTime()const; 
    unsigned short GetTrailingCoarseTimeSelector()const; 

  
    unsigned short GetLeadingFineTime()const; 
    unsigned short GetLeadingCoarseTime()const; 
    unsigned short GetLeadingCoarseTimeSelector()const; 


    unsigned short GetHitAddress()const;
    unsigned short GetPileUpAddress()const;
  
    unsigned short GetPixelGroupAddress()const;
    unsigned short GetQChip()const;
    void SetQChip(unsigned short qchip);

    unsigned short GetGTK()const;
    void SetGTK(unsigned short gtk);

    unsigned short GetChipId()const;
    void SetChipId(unsigned short chipid);
  
    unsigned int GetNaturalPixelIndex()const; 
    unsigned int GetNaturalColumnIndex()const; 
    //    unsigned int GetPixelUID()const; 
    int GetPixelUID()const;

    /* status accessors */
 
    unsigned short GetHitCount()const; 
    unsigned short GetQChipCollisionCount()const; 
    unsigned short GetQChipAddress()const;
  
    /* */
  
    unsigned char Get6bitWord(unsigned int idx)const;

    const unsigned char * GetBuffer()const;
    int SetBuffer(const unsigned char * buf);
    void SetFrameCount(unsigned int fc); 
  
  
    /* */
    unsigned int GetFrameCount()const; 


    friend std::ostream & operator<<(std::ostream & os, 
				     const TDCPixTimeStamp & ts); 
  

  private: 
    unsigned short mQChip;
    unsigned short mChipId;
    unsigned short mGTK;
    int mIsSerialFragment;
    unsigned int mFrameCount;
    double mLeadingTime;
    unsigned char mBuf[TDCPIX_TS_BUFFER_SIZE];
  
  };


  std::ostream & operator<<(std::ostream & os, 
			    const TDCPixTimeStamp & ts); 

  std::ostream & operator<<(std::ostream & os, 
			    const std::vector<TDCPixTimeStamp> & vts); 


  
  int is_time_stamp(const TDCPixTimeStamp & ts);



  //  int ExtractTimeStamps(const Packet32 & pkt, std::vector<TDCPixTimeStamp> & v);
  

}

#endif  
