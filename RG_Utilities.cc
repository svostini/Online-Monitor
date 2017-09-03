#include "RG_Utilities.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sstream> 
#include <cstring>
#include <fcntl.h>
#include <errno.h>

int RG_DirectoryExists(const char * aDir)
{
  struct stat lbuf;
  int lRet=stat(aDir, &lbuf);
  if(lRet!=-1){
    return S_ISDIR(lbuf.st_mode);//==S_IFREG;
  }
  return -1;
}

int RG_FileExists(const char * aFileName)
{
  struct stat lbuf;
  int lRet=stat(aFileName, &lbuf);
  if(lRet!=-1){
    return S_ISREG(lbuf.st_mode);//==S_IFREG;
  }
  return -1;
}

int RG_FIFOExists(const char * aFileName)
{
  struct stat lbuf;
  int lRet=stat(aFileName, &lbuf);
  if(lRet!=-1){
    return S_ISFIFO(lbuf.st_mode);//==S_IFREG;
  }
  return -1;
}

void rg_thread_sleep(long int nsec) {
  timespec delay = { nsec / 1000000000, nsec % 1000000000 };
  pselect(0, NULL, NULL, NULL, &delay, NULL);
}


void RG_Sleep(unsigned long ams)
{
  rg_thread_sleep(ams*1000000);
}


std::vector<std::string> RG_Tokenize(std::string input, char delimiter){
  std::stringstream ss( input.c_str());
  std::string s;
  std::vector<std::string> tokens;
  while (getline(ss, s, delimiter)) {
    tokens.push_back(s);
  }
  return tokens;
}

unsigned int RG_LargerOf(unsigned int a, 
			 unsigned int b)
{
  if(a>b){
    return a; 
  }
  return b; 
}




unsigned int binary_to_gray(unsigned int num)
{
  return (num>>1)^num;
}
 

unsigned int gray_to_binary(unsigned int num)
{
  unsigned int numBits = 8*sizeof(num);
  unsigned int shift=0;
  for (shift=1; shift<numBits; shift=2*shift)
    {
      num=num^(num>>shift);
    }
  return num;
}


unsigned int ones_count(unsigned int num, unsigned int nbits){
  unsigned int count=0; 
  for(unsigned int i(0); i!=nbits; i++){
    unsigned int bit=1<<i; 
    if(bit&num)count++;
  }
  return count;   
}
unsigned int parity(unsigned int num, unsigned int nbits)
{
  return (ones_count(num, nbits)&1);
}



int RG_DivertStdStreamToFile(int stream, std::string fname){

  int fd;
  int defout;
  if ((defout = dup(stream)) < 0)
    {
      fprintf(stderr, "Can't dup(2) - (%s)\n", std::strerror(errno));
      return(1);
    }
  //  if ((fd = open(fname.c_str(), O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | O_APPEND)) < 0)
  if ((fd = open(fname.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | O_APPEND)) < 0)
    {
      fprintf(stderr, "Can't open(2) - (%s)\n", std::strerror(errno));
      return(1);
    }
  if (dup2(fd, stream) < 0) // redirect output to the file
    {
      fprintf(stderr, "Can't dup2(2) - (%s)\n", std::strerror(errno));
      return(1);
    }
  close(fd);  // Descriptor no longer needed
  return defout;

}
int RG_DivertBackStdStream(int stream, int file){
  fflush(stdout);          // FLUSH ALL OUTPUT TO "out.txt"
  // Now stdout is clean for another target
  if (dup2(file, stream) < 0) // redirect output back to stdout
    {
      fprintf(stderr, "Can't dup2(2) - (%s)\n", std::strerror(errno));
      exit(1);
    }
  close(file);  // Copy of stdout no longer needed
  
  if (printf("") < 0)
    {
      fprintf(stderr, "Can't printf(3) to fd=%d - (%s)\n", fileno(stdout), std::strerror(errno));
      exit(1);
    }
  
}
