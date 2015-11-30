#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdlib>   

#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#endif

#include "udp.h"
#include "stun.h"
#include "stuntest.h"

using namespace std;

int stuntest(char* serverAddress)
{
   StunAddress4 stunServerAddr;
   stunServerAddr.addr=0;

   int srcPort = stunRandomPort();
   StunAddress4 sAddr;

   if (!stunParseServerName(serverAddress, stunServerAddr)) {
      return -1;
   }

   int retval = 0;

   sAddr.addr=0; 
   sAddr.port=0; 

   sAddr.port=srcPort;

   bool presPort=false;
   bool hairpin=false;
   bool verbose=false;
   
   NatType stype = stunNatType( stunServerAddr, verbose, &presPort, &hairpin, 
                                srcPort, &sAddr);
   switch (stype)
   {
      case StunTypeFailure:
         retval = -1;
         break;
      case StunTypeUnknown:
         retval = 0xEE;
         break;
      case StunTypeOpen:
         retval = 0x00; 
         break;
      case StunTypeIndependentFilter:
         retval = 0x02;
         break;
      case StunTypeDependentFilter:
         retval = 0x04;
         break;
      case StunTypePortDependedFilter:
         retval = 0x06;
         break;
      case StunTypeDependentMapping:
         retval = 0x08;
         break;
      case StunTypeFirewall:
         retval = 0x0A;
         break;
      case StunTypeBlocked:
         retval = 0x0C;
         break;
      default:
         retval = 0x0E;  // Unknown NAT type
         break;
   }
   if (!hairpin)
   {
      retval |= 0x10;
   }       

   if (presPort)
   {
      retval |= 0x01;
   }
   
   return retval;
}
