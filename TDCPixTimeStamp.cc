#include "TDCPixTimeStamp.hh"

#include <string.h>

#include "RG_Utilities.hh"

#include "Packet32.hh"

namespace GTK{



  int ConvertUID( unsigned int uid, /* 0 -> 18,000 */
		  /* outputs */
		  unsigned int & chip,
		  unsigned int & column,
		  unsigned int & pixel)
  {
    unsigned int x;
    if ( uid<9000 ){
      pixel    = uid/200;
      x = uid%200;
      chip = 4 - x/40;
      column = x%40;
    }
    else{
      unsigned int m_uid = uid = 9000;
      pixel    = 44 - m_uid/200;      
      x = m_uid%200;
      chip = 5 + x/40;
      column = 39 - x%40;
    }
    return 0;
  }



  int ConvertMatrixIndices(unsigned int column, /* 0-39 */
			   unsigned int pixel,  /* 0-44 */
			   /* outputs */
			   unsigned int & qchip,
			   unsigned int & pixel_group_address,
			   unsigned int & hit_arbiter_address)
  {

    unsigned int r_pixel=45-pixel-1;

    qchip=column/10;

    pixel_group_address=(column%10)*9+(r_pixel%9);

    unsigned int hit_arbiter_idx=(r_pixel)/9;
    hit_arbiter_idx=(1<<hit_arbiter_idx);

    hit_arbiter_address = hit_arbiter_idx;
    return 0;
  }



  int ConvertMatrixIndices(unsigned int column_pair, /* 0-19 */
			   unsigned int odd_col,     /* 0-1  */
			   unsigned int pixel,       /* 0-44 */
			   /* outputs */
			   unsigned int & qchip,
			   unsigned int & pixel_group_address,
			   unsigned int & hit_arbiter_address)
  {
    unsigned int column=2*column_pair+odd_col;
    return ConvertMatrixIndices(column, pixel, qchip, pixel_group_address, hit_arbiter_address);
  }



  int ConvertTimeStampIndices(unsigned int qchip,
			      unsigned int pixel_group_address,
			      unsigned int hit_arbiter_address,
			      /* outputs */
			      unsigned int & column,
			      unsigned int & pixel)
  {
    //column=qchip*10+pixel_group_address/9;

    if(qchip == 0) qchip =3;
    else if(qchip == 3) qchip =0;
    else if(qchip == 1) qchip =2;
    else if(qchip == 2) qchip =1;

    column= qchip*10+(9-pixel_group_address/9);
    unsigned int hit_arbiter_idx=0;
    switch(hit_arbiter_address){
    case 1:
      hit_arbiter_idx=0;
      break;
    case 2:
      hit_arbiter_idx=1;
      break;
    case 4:
      hit_arbiter_idx=2;
      break;
    case 8:
      hit_arbiter_idx=3;
      break;
    case 16:
      hit_arbiter_idx=4;
      break;
    default:
      return -1;
    };

    unsigned int r_pixel=pixel_group_address%9+hit_arbiter_idx*9;
    //pixel=45-r_pixel-1;
    pixel=r_pixel;
    return 0;
  }




  int ConvertTimeStampIndices(unsigned int qchip,
			      unsigned int pixel_group_address,
			      unsigned int hit_arbiter_address,
			      /* outputs */
			      unsigned int & column_pair,
			      unsigned int & odd_col,
			      unsigned int & pixel)
  {
    unsigned int column=0;
    int rv=ConvertTimeStampIndices(qchip, pixel_group_address, hit_arbiter_address,
				   column, pixel);
    if(rv){return rv;}

    column_pair=column/2;
    odd_col=column%2;
    return 0;
  }




  TDCPixTimeStamp::TDCPixTimeStamp(int serial_fragment):
    mQChip(0),
    mLeadingTime(0),
    mIsSerialFragment(serial_fragment),
    mFrameCount(0)
  {
    memset(mBuf, 0, TDCPIX_TS_BUFFER_SIZE);
  }

  TDCPixTimeStamp::~TDCPixTimeStamp()
  {

  }


  int TDCPixTimeStamp::Clear()
  {
    mQChip=0;
    mLeadingTime = 0;
    mIsSerialFragment=0;
    mFrameCount=0;

    memset(mBuf, 0, TDCPIX_TS_BUFFER_SIZE);
    return 0;
  }

  const double TDCPixTimeStamp::ClockPeriod = 24.951059536;

  double TDCPixTimeStamp::ComputeLeadingTime(const double& offset, const bool& fCorr)
  {
    unsigned short ft=gray_to_binary(this->GetLeadingFineTime());
    unsigned short ct=this->GetLeadingCoarseTime();
    unsigned short ct_bin=gray_to_binary(ct);


    if(ft>20){// && ft<30){// use disambiguity mechanism:
      int par=parity(ct, 12);
      int sel=this->GetLeadingCoarseTimeSelector();
      if(par!=sel){// parity doesn't match selector:
	ct_bin = (ct_bin-1)&0xfff;  //what to do it ct == 0 => ct_bin = 0xfff
      }
    }


    // basic calculation
    double time=double( (ct_bin&0x7ff) )*ClockPeriod/8.0;// ps/clk cycle
    time+=(double(ft)+0.5)*ClockPeriod/256.0;//=97.65625/2.0;// bin centre:

    unsigned int fc = this->GetFrameCount();
    //  Due to queueing some time stamps cannot be sent in the frame they belong to
    //  and get sent only in the next frame, so the frame counter has to be corrected.
    if( ((fc&0x1) !=  (ct_bin&0x800)>>11) && fCorr == 1) {
      fc = fc-1;
    }

    time += double(fc)*ClockPeriod*256.0; // ps/frame
    time += offset;

    mLeadingTime = time;
    return time;
  }

  double TDCPixTimeStamp::GetLeadingTime( bool fCorr ) const
   {
    unsigned short ft=gray_to_binary(this->GetLeadingFineTime());
    unsigned short ct=this->GetLeadingCoarseTime();
    unsigned short ct_bin=gray_to_binary(ct);


    if(ft>20){// && ft<30){// use disambiguity mechanism:
      int par=parity(ct, 12);
      int sel=this->GetLeadingCoarseTimeSelector();
      if(par!=sel){// parity doesn't match selector:
	ct_bin = (ct_bin-1)&0xfff;  //what to do it ct == 0 => ct_bin = 0xfff
      }
    }


    // basic calculation
    double time=double( (ct_bin&0x7ff) )*ClockPeriod/8.0;// ps/clk cycle
    time+=(double(ft)+0.5)*ClockPeriod/256.0;//=97.65625/2.0;// bin centre:

    unsigned int fc = this->GetFrameCount();
    //  Due to queueing some time stamps cannot be sent in the frame they belong to
    //  and get sent only in the next frame, so the frame counter has to be corrected.
    if( ((fc&0x1) !=  (ct_bin&0x800)>>11) && fCorr == 1) {
      fc = fc-1;
    }

    time += double(fc)*ClockPeriod*256.0; // ps/frame
    return time;
  }


  double TDCPixTimeStamp::GetTrailingTime(bool fCorr ) const
  {
    // First find the frame to which the hit leadind time belong
    unsigned short ft_lead=gray_to_binary(this->GetLeadingFineTime());
    unsigned short ct_lead=this->GetLeadingCoarseTime();
    unsigned short ct_lead_bin=gray_to_binary(ct_lead);
    unsigned int lead_corr(0);

    if(ft_lead>20){// && ft<30){// use disambiguity mechanism:
      int par=parity(ct_lead, 12);
      int sel=this->GetLeadingCoarseTimeSelector();
      if(par!=sel){// parity doesn't match selector:
    	ct_lead_bin = (ct_lead_bin-1)&0xfff;
	lead_corr = 1;
      }
    }
    unsigned int fc = this->GetFrameCount();
    if( ((fc&0x1) !=  (ct_lead_bin&0x800)>>11) && fCorr == 1)  fc = fc-1;

    // The trailing coarse counter should counts from the begining of the
    // leading frame. If the trailing falls in the next frame then
    // the coarse time should count over 4095.
    // In particular if raw_cc = 0 and the disambiguation puts it to 4094
    // the trailing coarse time  should be 4094 + tot and not 0+tot.
    // So the following has to be done:
    ct_lead_bin = ct_lead_bin + lead_corr;

    // Second compute the coarse trailing time
    unsigned short raw_ct_lead_bin=gray_to_binary(ct_lead); //reset to original value
    unsigned short tot=this->GetTrailingCoarseTime();
    unsigned short ct_trail_bin=(tot+raw_ct_lead_bin)&0xfff;
    unsigned short ct_trail=binary_to_gray(ct_trail_bin);
    unsigned short ft_trail=gray_to_binary(this->GetTrailingFineTime());

    ct_trail_bin = ct_lead_bin+tot;
    if(ft_trail>20){
      int par=parity(ct_trail,12);
      int sel=this->GetTrailingCoarseTimeSelector();
      if(par!=sel){// parity doesn't match selector:
	ct_trail_bin--;
      }
    }
    double time=double(ct_trail_bin)*ClockPeriod/8.0;
    time+=double((ft_trail)+0.5)*ClockPeriod/256.0;
    time += double(fc&0xfffffffe)*ClockPeriod*256.0;

    return time;
  }



  unsigned short TDCPixTimeStamp::IsTimeStamp()const
  {
    /* bit 47 : 1 => frame synch word; 0 => time stamp */


    return (mBuf[5]&0x80)?0:1; //most significant bit of the 48bit word is 0 for TS
    //  unsigned short frame_word_markers=(mBuf[5]>>5)&0x7;
    //   switch(frame_word_markers){
    //   case 0:
    //   case 1:
    //   case 2:
    //   case 4:
    //     return 1; /* these cases equate to a majority 0*/
    //     break;
    //   case 3:
    //   case 5:
    //   case 6:
    //   case 7:
    //     return 0; /* these cases equate to a */
    //     break;
    //   };

    //   RG_REPORT_ERROR_MSG_CODE("Invalid state reached: ", frame_word_markers);
    //   return 0;
    //return (mBuf[5]&0xe0)?0:1;
  }


  unsigned short TDCPixTimeStamp::IsPaddingWord()const
  {
    const static unsigned char padding[6]={0xff,0xff,0xff,0xff,0xff,0xff};
    int rv=strncmp(reinterpret_cast<const char *>(mBuf),
		   reinterpret_cast<const char *>(padding),
		   6);
    return rv?0:1;
  }



  unsigned short TDCPixTimeStamp::GetTrailingFineTime()const{
    /* 4:0 */
    return 0x1f&mBuf[0];
  }

  unsigned short TDCPixTimeStamp::GetTrailingCoarseTime()const{
    /* 10:5 */
    unsigned short temp0=mBuf[1]&0x7; temp0<<=3;
    unsigned short temp1=(mBuf[0]>>5)&0x7;

    return temp0|temp1;
  }
  unsigned short TDCPixTimeStamp::GetTrailingCoarseTimeSelector()const{
    /* bit 11 */
    return (0x8&mBuf[1])?1:0;
  }

  unsigned short TDCPixTimeStamp::GetLeadingFineTime()const{
    /* 16:12 */
    unsigned short temp0=(mBuf[2]<<4)&0x10;
    unsigned short temp1=(mBuf[1]>>4)&0xf;
    return temp0|temp1;
  }
  unsigned short TDCPixTimeStamp::GetLeadingCoarseTime()const
  {
    /* 28:17 */
    unsigned short temp0=mBuf[3]&0x1f;temp0<<=7;
    unsigned short temp1=(mBuf[2]>>1)&0x7f;
    return temp1|temp0;
  }
  unsigned short TDCPixTimeStamp::GetLeadingCoarseTimeSelector()const{
    /* bit 29 */
    return (mBuf[3]>>5)&1;
  }


  unsigned short TDCPixTimeStamp::GetHitAddress()const{
    /* bits 39:35 */
    return 0x1f&(mBuf[4]>>3);
  }
  unsigned short TDCPixTimeStamp::GetPileUpAddress()const{
    /* bits 34:30 */
    unsigned short temp0=mBuf[4]&0x7; temp0<<=2;
    unsigned short temp1=(mBuf[3]>>6)&0x3;
    return temp1|temp0;
  }

  unsigned short TDCPixTimeStamp::GetPixelGroupAddress()const
  {
    /* bits 46:40 */
    return mBuf[5]&0x7f;
  }


  // there is no byte 7 any more; this was from the sim:
  unsigned short TDCPixTimeStamp::GetQChip()const
  {
    return mQChip;//mBuf[7]&0x3;
  }

  void TDCPixTimeStamp::SetQChip(unsigned short qchip)
  {
    mQChip=qchip;
  }

  unsigned short TDCPixTimeStamp::GetGTK()const
  {
    return mGTK;
  }

  void TDCPixTimeStamp::SetGTK(unsigned short gtk)
  {
    mGTK=gtk;
  }



  unsigned short TDCPixTimeStamp::GetChipId()const
  {
    return mChipId;//mBuf[7]&0x3;
    //    return mBuf[7]&0x3;
  }

  void TDCPixTimeStamp::SetChipId(unsigned short chipid)
  {
    mChipId=chipid;
  }


  unsigned int TDCPixTimeStamp::GetNaturalPixelIndex()const
  {
    unsigned short pga=GetPixelGroupAddress();
    unsigned short haa=GetHitAddress();
    unsigned short qchip=GetQChip();

    unsigned int column;
    unsigned int pixel;
    ConvertTimeStampIndices(qchip, pga, haa,
			    /* outputs */
			    column, pixel);

    return pixel;
  }
  unsigned int TDCPixTimeStamp::GetNaturalColumnIndex()const
  {
    unsigned short pga=GetPixelGroupAddress();
    unsigned short haa=GetHitAddress();
    unsigned short qchip=GetQChip();

    unsigned int column;
    unsigned int pixel;
    ConvertTimeStampIndices(qchip, pga, haa,
			    /* outputs */
			    column, pixel);

    return column;
  }

  /*
  unsigned int TDCPixTimeStamp::GetPixelUID()const
  {
    unsigned short pga=GetPixelGroupAddress();
    unsigned short haa=GetHitAddress();
    unsigned short qchip=GetQChip();

    unsigned int column;
    unsigned int pixel;
    ConvertTimeStampIndices(qchip, pga, haa,
			    column, pixel);
    return pixel*40 + column;
  }*/

  /* status accessors: */


  unsigned short TDCPixTimeStamp::GetHitCount()const{
    if(IsTimeStamp()){
      RG_REPORT_ERROR_MSG("time stamp is not a frame marker.");
      //      std::cout<<(*this)<<std::endl;
      return 9999;
    }
    /* 36:28 */
    unsigned short temp0=0x1f&mBuf[4];temp0<<=4; //bug in Matt's code was temp0<<=5;
    unsigned short temp1=(mBuf[3]>>4)&0xf;
    return temp1|temp0;
  }

  unsigned short TDCPixTimeStamp::GetQChipCollisionCount()const{
    /* 42:37 */
    unsigned short temp0=0x7&mBuf[5]; temp0<<=3;
    unsigned short temp1=(mBuf[4]>>5)&0x7;
    return temp0|temp1;
  }

  unsigned short TDCPixTimeStamp::GetQChipAddress()const
  {
    if(IsTimeStamp()){
      RG_REPORT_ERROR_MSG("time stamp is not a frame marker.");
      return 0;
    }

    unsigned short temp=(mBuf[5]>>3)&0x3;
    return temp;
  }



  unsigned char TDCPixTimeStamp::Get6bitWord(unsigned int idx)const{
    /* idx must be less than 8 */
    char ebuf[128];
    unsigned char temp=0;
    unsigned char temp2=0;
    switch(idx){
    case 0:
      temp=mBuf[0]&0x3f;
      break;
    case 1:
      temp2=mBuf[1]&0xf; temp2<<=2;
      temp=mBuf[0]&0xc0; temp>>=6;
      temp=temp|temp2;
      break;
    case 2:
      temp2=mBuf[2]&0x3; temp2<<=4;
      temp=mBuf[1]&0xf0; temp>>=4;
      temp=temp|temp2;
      break;
    case 3:
      temp=(mBuf[2]>>2)&0x3f;
      break;
    case 4:
      temp=mBuf[3]&0x3f;
      break;
    case 5:
      temp2=mBuf[4]&0xf; temp2<<=2;
      temp=mBuf[3]&0xc0; temp>>=6;
      temp=temp|temp2;
      break;
    case 6:
      temp2=mBuf[5]&0x3; temp2<<=4;
      temp=mBuf[4]&0xf0; temp>>=4;
      temp=temp|temp2;
      break;
    case 7:
      temp=(mBuf[5]>>2)&0x3f;
      break;
    default:
      snprintf(ebuf, 120, "%d",idx);
      RG_REPORT_ERROR_MSG2("invalid index passed to function: ", ebuf);
      return 0xff;
      break;
    };

    return temp;
  }



  /*  */
  const unsigned char * TDCPixTimeStamp::GetBuffer()const{
    return mBuf;
  }

  int TDCPixTimeStamp::SetBuffer(const unsigned char * buf){
    memcpy(mBuf, buf, TDCPIX_TS_BUFFER_SIZE);
    // for(unsigned int i(0);i!=TDCPIX_TS_BUFFER_SIZE; i++){
    //   mBuf[i]=buf[TDCPIX_TS_BUFFER_SIZE-1-i];
    // }
    return 0;
  }

  void TDCPixTimeStamp::SetFrameCount(unsigned int fc){
    mFrameCount=fc;
  }


  unsigned int TDCPixTimeStamp::GetFrameCount()const{
    if(IsTimeStamp()){
      return mFrameCount;
    }
    /* bits 27:0 */
    //return reinterpret_cast<const unsigned int *>(mBuf)[0]&0x0ffffff;
    return reinterpret_cast<const unsigned int *>(mBuf)[0]&0x0fffffff;
  }

  int TDCPixTimeStamp::GetPixelUID()const{
    //Chips in GTK are labelled as:
    //  [for Sensor Upstream]
    //      ┌─┬─┬─┬─┬─┐
    //      │5│6│7│8│9│
    // Jura ├─┼─┼─┼─┼─┤ Saleve
    //      │0│1│2│3│4│
    //      └─┴─┴─┴─┴─┘
    //
    unsigned int qchip = this->GetQChip();
    unsigned int pixel_group_address = this->GetPixelGroupAddress();
    unsigned int hit_arbiter_address = this->GetHitAddress();
    if(qchip == 0) qchip =2;
    else if(qchip == 1) qchip =0;
    else if(qchip == 2) qchip =1;
    else if(qchip == 3) qchip =3;
    unsigned int x = qchip*10+(9-pixel_group_address/9);
    unsigned int hit_arbiter_idx=0;
    switch(hit_arbiter_address){
    case 1:
      hit_arbiter_idx=0;
      break;
    case 2:
      hit_arbiter_idx=1;
      break;
    case 4:
      hit_arbiter_idx=2;
      break;
    case 8:
      hit_arbiter_idx=3;
      break;
    case 16:
      hit_arbiter_idx=4;
      break;
    default:
      return -1;
    };
    unsigned int y = pixel_group_address%9+hit_arbiter_idx*9;
    unsigned int chipid  = this->GetChipId()-1;
    if(chipid<4){
      x = 39-x;
      y = 44-y;
    }
    // if (!this->GetSensorUpstream()) {
    //    chipid = (chipid+5)%10;
    // }
    // else  y = 44-y;
    y = 44-y;
    int UID = x+chipid%5*40 + y*200 + chipid/5 * 200*45;
    //    printf("Pixel UID: %d  x: %u y:%u  chipId: %u\n",UID,x,y,chipid);
    return UID;
  }

  std::ostream & operator<<(std::ostream & os,
			    const TDCPixTimeStamp & ts)
  {
    int channel=ts.mBuf[TDCPIX_TS_BUFFER_SIZE-1];

    // os<<"-----------------------------------------------\n";
    if(ts.IsTimeStamp()){
      // this is a real time stamp:
      // os<<"\n\n channel = "<<channel<<" : \n";
      os<<" time stamp data = ";
      os<<"0x";
      for(unsigned int i(0); i!=TDCPIX_TS_BUFFER_SIZE; i++){
	int temp=ts.mBuf[TDCPIX_TS_BUFFER_SIZE-i-1];
	os.width(2);os.fill('0');
	os<<std::hex<<temp;
      }

      os<<std::dec;

      os<<"\n";
      os<<"   qchip address         = "<<ts.GetQChip()<<"\n";
      os<<"   pixel address         = "<<ts.GetPixelGroupAddress()<<"\n";

      os<<"   hit address           = "<<ts.GetHitAddress()<<"\n";
      os<<"   pile up address       = "<<ts.GetPileUpAddress()<<"\n";

      os<<"   frame counter         = "<<ts.GetFrameCount()<<"\n";

      os<<"   leading fine time     = "<<ts.GetLeadingFineTime()<<"\n";
      os<<"   leading coarse time   = "<<ts.GetLeadingCoarseTime()<<"[g] -> "<<gray_to_binary(ts.GetLeadingCoarseTime())<<"[b]\n";
      os<<"   leading ambiguity bit = "<<ts.GetLeadingCoarseTimeSelector()<<"\n";
      os<<"   leading ct parity     = "<<parity(ts.GetLeadingCoarseTime(), 12)<<"\n";


      os<<"   trailing fine time    = "<<ts.GetTrailingFineTime()<<"\n";
      os<<"   trailing coarse time  = "<<ts.GetTrailingCoarseTime()<<"\n";// -> "<<gray_to_binary(ts.GetTrailingCoarseTime())<<"\n";
      os<<"   trailing ambiguity bit= "<<ts.GetTrailingCoarseTimeSelector()<<"\n";

      os<<"     row    = "<<ts.GetNaturalPixelIndex()<<"\n";
      os<<"     column = "<<ts.GetNaturalColumnIndex()<<"\n";


    }
    else{
      // this is a frame sync word or a test word:
      os<<"   Frame Count          = "<<std::dec<<ts.GetFrameCount()<<"\n";

      //os<<"\n\n channel = "<<channel<<" : \n";
      os<<" frame sync word = ";
      os<<"  0x";
      for(unsigned int i(0); i!=TDCPIX_TS_BUFFER_SIZE; i++){
	int temp=ts.mBuf[TDCPIX_TS_BUFFER_SIZE-i-1];
	os.width(2);
	os.fill('0');
	os<<std::hex<<temp<<' ';
      }
      os<<"\n";
      os<<"   hit count            = "<<std::dec<<ts.GetHitCount() <<"\n";
      os<<"   collision count      = "<<std::dec<<ts.GetQChipCollisionCount()<<"\n";
      os<<"   Frame Count          = "<<std::dec<<ts.GetFrameCount()<<"\n";

      }

    return os;
  }





  std::ostream & operator<<(std::ostream & os,
			    const std::vector<TDCPixTimeStamp> & vts)
  {

    std::vector<TDCPixTimeStamp>::const_iterator it=vts.begin();
    std::vector<TDCPixTimeStamp>::const_iterator end=vts.end();
    while(it!=end){
      os<<(*it);
      ++it;
    }

    return os;
  }





  int is_time_stamp(const TDCPixTimeStamp & ts)
  {
    return ts.IsTimeStamp();
  }






  /* should use the TDCPixTimeStampCollection instead: */
  // int ExtractTimeStamps(const Packet32 & pkt, std::vector<TDCPixTimeStamp> & v)
  // {

  //   v.clear();

  //   const unsigned int * payload=pkt.GetPayload();
  //   const unsigned char * p=reinterpret_cast<const unsigned char *>(payload);
  //   u32 len=(pkt.GetPayloadLength()-1)*4;
  //   u32 nts=(len)/6;/* number of timestamps */
  //   v.reserve(nts);

  //   unsigned int ts_offset=0;
  //   for(unsigned int i(0); i!=nts; i+=2, ts_offset+=12){
  //     unsigned char buf_even[6];
  //     unsigned char buf_odd[6];

  //     buf_even[5]=p[3+ts_offset];
  //     buf_even[4]=p[2+ts_offset];
  //     buf_even[3]=p[1+ts_offset];
  //     buf_even[2]=p[0+ts_offset];
  //     buf_even[1]=p[7+ts_offset];
  //     buf_even[0]=p[6+ts_offset];


  //     buf_odd[5]=p[5+ts_offset];
  //     buf_odd[4]=p[4+ts_offset];
  //     buf_odd[3]=p[11+ts_offset];
  //     buf_odd[2]=p[10+ts_offset];
  //     buf_odd[1]=p[9+ts_offset];
  //     buf_odd[0]=p[8+ts_offset];


  //     TDCPixTimeStamp ts;
  //     ts.SetBuffer(buf_even);
  //     if(!ts.IsPaddingWord()){// test for validity:
  // 	v.push_back(ts);
  //     }
  //     ts.SetBuffer(buf_odd);
  //     if(!ts.IsPaddingWord()){// test for validity:
  // 	v.push_back(ts);
  //     }
  //   }

  //   return 0;
  // }




}
