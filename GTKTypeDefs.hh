#ifndef GTKTypeDefs_HH_
#define GTKTypeDefs_HH_

#include <iostream>
#include <stdlib.h>


/*
  
 *
 */

namespace GTK
{

  typedef signed char    i8;  // Signed 8-bit integer.
  typedef signed short   i16; // Signed 16-bit integer.
  typedef int            i32; // Signed 32-bit integer.
  
  typedef unsigned char  u8;  // Unsigned 8-bit integer.

#ifndef _MYUINT_H
typedef unsigned short u16; // Unsigned 16-bit integer.
typedef unsigned int   u32; // Unsigned 32-bit integer.
#define _MYUINT_H
#endif

//  typedef unsigned short u16; // Unsigned 16-bit integer.
//  typedef unsigned int   u32; // Unsigned 32-bit integer.
  



  struct SizeCheck{
    SizeCheck():
      mI8Size(1), 
      mI16Size(2), 
      mI32Size(4), 
      mU8Size(1), 
      mU16Size(2), 
      mU32Size(4)    
    {
      bool lExit=false;
      if(sizeof(i8)!=mI8Size){
	std::cerr<<__PRETTY_FUNCTION__<<" GTK::i8 is wrong size\n";
	lExit=true;
      }

      if(sizeof(i16)!=mI16Size){
	std::cerr<<__PRETTY_FUNCTION__<<" GTK::i16 is wrong size\n";
	lExit=true;
      }
  
      if(sizeof(i32)!=mI32Size){
	std::cerr<<__PRETTY_FUNCTION__<<" GTK::i32 is wrong size\n";
	lExit=true;
      }

      if(sizeof(u8)!=mU8Size){
	std::cerr<<__PRETTY_FUNCTION__<<" GTK::u8 is wrong size\n";
	lExit=true;
      }

      if(sizeof(u16)!=mI16Size){
	std::cerr<<__PRETTY_FUNCTION__<<" GTK::u16 is wrong size\n";
	lExit=true;
      }


      if(sizeof(u32)!=mU32Size){
	std::cerr<<__PRETTY_FUNCTION__<<" GTK::u32 is wrong size\n";
	lExit=true;
      }
      
      
      if(lExit){
	std::cerr<<"*** Edit file: "<<__FILE__<<std::endl;
	
	exit(1);
      }
    }

    int mI8Size;
    int mI16Size;
    int mI32Size;

    int mU8Size;
    int mU16Size;
    int mU32Size;
  
  };

  //SizeCheck gSizeCheck__;

}//~namespace GTK

#endif //~GTKTypeDefs_HH_
