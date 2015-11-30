#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_APPLICATION

/**
 * This is where the platform and uNabto is initialized and ticked.
 * It uses the encryption key, id and mac from the bootloader data area
 */

#include <unabto/unabto.h>
#include <bootloader_base.h>
#include "hal.h"
#include "application.h"

#define SUPPORT_BOOTLOADER_VERSION_1 1

#if SUPPORT_BOOTLOADER_VERSION_1

// this structure is only used on very early boards with the very first bootloader
// as this version of the bootloader did not contain the shared secret and the product domain.

typedef struct
{
  const uint8_t sharedSecret[16];
  const char productDomain[100];
  const char dummy[2048 - 16 - 100];
} application_data_version1;

// import shared secret and product domain from application data area
#pragma romdata application_data_section = 0x1f400
const far rom application_data_version1 applicationData;
#pragma romdata

#endif

// <editor-fold defaultstate="collapsed" desc="Interrupts">

//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#pragma interruptlow LowISR

void LowISR(void)
{
  TickUpdate();
}

#pragma interruptlow HighISR

void HighISR(void)
{
}

#pragma code lowVector=0x06018

void LowVector(void)
{
  _asm
  goto LowISR
    _endasm
}
#pragma code

#pragma code highVector=0x06008

void HighVector(void)
{
  _asm
  goto HighISR
    _endasm
}
#pragma code

// </editor-fold>

void main(void)
{
  static nabto_main_setup* nms;
  static char versionString[NABTO_DEVICE_VERSION_MAX_SIZE];
  static char idString[NABTO_DEVICE_NAME_MAX_SIZE];
#if NABTO_ENABLE_UCRYPTO
  static const far rom uint8_t dummySharedSecret[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static const far rom uint8_t* sharedSecret;
#endif

  // <editor-fold desc="Load information from bootloader and application data areas.">

  // Clear the global TCP/IP data structure and load it with the boards unique preprogrammed MAC address.
  memset((void*) &AppConfig, 0x00, sizeof (AppConfig));
  memcpypgm2ram(&AppConfig.MyMACAddr, (const __ROM uint8_t*) bootloaderData.version1.mac, 6);

  if(bootloaderData.base.bootloaderDataVersion == 1)
  {
    strcpypgm2ram(idString, (const far rom char*) bootloaderData.version1.serialNumber);
    strcatpgm2ram(idString, ".nabduino.net");

#if NABTO_ENABLE_UCRYPTO
    //        sharedSecret = applicationData.sharedSecret; // the old pre-bootloader-version-2 way of storing the shared secret - obsolete!
#endif
  }
  else if(bootloaderData.base.bootloaderDataVersion == 2)
  {
    hardwareVersion = (uint8_t) bootloaderData.version2.hardwareVersionMajor;
    hardwareVersion <<= 8;
    hardwareVersion |= (uint8_t) bootloaderData.version2.hardwareVersionMinor;

    if(hardwareVersion == 0x0004)
    {
      hardwareVersionIndex = 0; // first version of the board that was released.
    }
    else if(hardwareVersion == 0x0102)
    {
      hardwareVersionIndex = 1; // second version of the board that was released (yes we jumped from 0.4 beta to 1.2).
    }
    else if(hardwareVersion == 0x0103)
    {
      hardwareVersionIndex = 2; // third version of the board
    }

    strcpypgm2ram(idString, (const far rom char*) bootloaderData.version2.deviceId);
    strcatpgm2ram(idString, (const far rom char*) bootloaderData.version2.productDomain);

#if NABTO_ENABLE_UCRYPTO
    sharedSecret = bootloaderData.version2.sharedSecret;
#endif
  }
  else
  { // unsupported version so it's probably safe to assume that bootloader data has been wiped - please fill in the appropriate values for your Nabduino board
    hardwareVersionIndex = 0; // hardware version 0.4 = 0, v1.2 = 1, v1.3 = 2

    AppConfig.MyMACAddr.v[0] = 0xBC; // Nabto owned MAC OUI is BC:A4:E1
    AppConfig.MyMACAddr.v[1] = 0xA4;
    AppConfig.MyMACAddr.v[2] = 0xE1;
    AppConfig.MyMACAddr.v[3] = 0x00;
    AppConfig.MyMACAddr.v[4] = 0x00;
    AppConfig.MyMACAddr.v[5] = 0x00; // If using more than one Nabduino on the same network make this byte unique for each board

    strcpypgm2ram(idString, "XXX"); // replace XXX with the id of the Nabduino board
    strcatpgm2ram(idString, ".nabduino.net");

#if NABTO_ENABLE_UCRYPTO
    sharedSecret = dummySharedSecret;
#endif
  }

  // </editor-fold>

  // Initialize IOs etc. taking into account the hardware version.
  hal_initialize();

  // Initialize the platform (timing, TCP/IP stack, DHCP and some PIC18 specific stuff)
  platform_initialize();

  network_initialize();

  nms = unabto_init_context();

  nms->id = (const char*) idString;

  // build version string: <application SVN version>/<bootloader SVN version>/<hardware major version>.<hardware minor version>
  itoa(RELEASE_MINOR, versionString);
  strcatpgm2ram(versionString + strlen(versionString), "/");
  itoa(bootloaderData.base.buildVersion, versionString + strlen(versionString));
  strcatpgm2ram(versionString + strlen(versionString), "/");
  itoa(hardwareVersion >> 8, versionString + strlen(versionString));
  strcatpgm2ram(versionString + strlen(versionString), ".");
  itoa(hardwareVersion & 0xff, versionString + strlen(versionString));
  nms->version = (const char*) versionString;

#if NABTO_ENABLE_UCRYPTO
  memcpypgm2ram(nms->presharedKey, (const __ROM void*) sharedSecret, 16);
  nms->secureAttach = true;
  nms->secureData = true;
  nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
#endif

  setup((char**) &nms->url);

  unabto_init();

  while(1)
  {
    hal_tick();
    platform_tick();
    network_tick();
    unabto_tick();
    loop();
  }
}
