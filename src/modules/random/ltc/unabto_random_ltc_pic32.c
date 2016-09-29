#include "tomcrypt.h"
#include <TCPIP Stack/TCPIP.h>
#include <TCPIP Stack/Helpers.h>

unsigned long rng_get_bytes(unsigned char *out, unsigned long outlen, 
                            void (*callback)(void))
{
   unsigned long x;

   LTC_ARGCHK(out != NULL);

   DWORD GenerateRandomDWORD();

   unsigned long missing = outlen;
   
   while (missing > 0) {
       DWORD rand = GenerateRandomDWORD();
       unsigned int copyLength = MIN(sizeof(rand), outlen);
       memcpy(out, &rand, copyLength);
       out+=copyLength;
       missing -= copyLength;
   }

   return outlen;
}

