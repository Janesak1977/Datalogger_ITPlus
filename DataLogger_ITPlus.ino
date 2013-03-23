/**
 * Temperature data logger.
 *
 * Main module
 *
 * Gérard Chevalier, Jan 2011
 * Nov 2011, changed to integrate La Crosse IT+ protocol
 */

#include "DataloggerDefs.h"
#include <avr/pgmspace.h>
#if 0
#include "dnslkup.h"
#endif

extern void RF12Init();
#if 0
extern void NetworkInitialize();
extern void WebSend();
extern void DecodeFrame();
extern void Get1820Tmp();
#endif
extern void CheckRF12Recept();
extern void DebugPrint_P(const char *);
extern void DebugPrintln_P(const char *);
#if 0
extern void CheckProcessBrowserRequest();
#endif

extern byte CentralTempSignBit, CentralTempWhole, CentralTempFract;
extern Type_Channel ITPlusChannels[];
extern Type_Discovered DiscoveredITPlus[];
//extern Type_Channel RF12Channels[];
extern Type_Config Config;
#if 0
extern word LastServerSendOK;
extern byte buf[];
extern byte DNSState;
#endif
boolean Acquire1820 = true;  // True to trigger a measure upon reset, and not wait first mn elapsed

boolean ANewMinute = false;
unsigned long previousMillis = 0;
// All time calculation done in mn with 8 bits, having so a roll-over of about 4 h
// But total nb of mn stored as 16 bits
byte LastWebSend = 0;
word Minutes = 0;
//byte WebSendPeriod;

// For seconds, 1 byte also used as we just want to check within 1 minute
byte Seconds = 0;

// "0,-tt.dNL1,-tt.dNL2,-tt.dNL3,-tt.dNL0" xxxxxxxxxxxxxxxx 0,-tt.dNL1,-tt.dNL2,-tt.dNL3,-tt.dNL4,-tt.dNL5,-tt.dNL6,-tt.dNL0
char CommonStrBuff[50], *PtData;

// SignalError tells if an error has to be signaled on LED, 0=No, 1&2=yes, 2 values for blinking
byte SignalError = 1;
boolean ErrorCondition = false;


/* main Initialization */
/* ------------------- */
void setup() {
  Serial.begin(57600);
  DebugPrintln_P(PSTR("Data Logger startup"));
  DDRC |= _BV(DDC3);  // LED Pin = output
  LoadConfig();
  // Found this comment into network library: "init ENC28J60, must be done after SPI has been properly set up!"
  // So taking care of calling RF12 initialization prior ethernet initialization
  RF12Init();
#if 0
  NetworkInitialize();
#endif
  ITPlusRXSetup();
#if 0
  DS1820Init();
#endif
  DebugPrintln_P(PSTR("Data Logger ready"));
}

void loop() {
  // Check if a new second elapsed
  if (millis() - previousMillis > 1000L) {
    previousMillis = millis();
    Seconds++;
    if (Seconds == 60) {
      Seconds = 0;
      Minutes++; ANewMinute = true;
    }
    // Error signaling code : must be run once per second.
    if (SignalError != 0) {
      if (SignalError == 1) {
        RX_LED_ON(); SignalError = 2;
      } else {
        RX_LED_OFF(); SignalError = 1;
      }
    }
  }

  // If one minute elapsed, check what we have to do
  if (ANewMinute) {
    ANewMinute = false;
    // Decrement LastReceiveTimer for all channels every mn...
//    for (byte Channel = 0; Channel < MAX_JEENODE; Channel++) {
//      if (RF12Channels[Channel].LastReceiveTimer != 0) RF12Channels[Channel].LastReceiveTimer--;
//    }
    for (byte Channel = 0; Channel < ITPLUS_MAX_SENSORS; Channel++) {
      if (ITPlusChannels[Channel].LastReceiveTimer != 0) ITPlusChannels[Channel].LastReceiveTimer--;
    }
    for (byte Channel = 0; Channel < ITPLUS_MAX_DISCOVER; Channel++) {
      if (DiscoveredITPlus[Channel].LastReceiveTimer != 0) DiscoveredITPlus[Channel].LastReceiveTimer--;
    }

    // To simplify, a new DS1820 measure is triggered every minute
    Acquire1820 = true;

#if 0
    // Reset DNS Lookup State Machine every mn if got no answer to force retry
    if (DNSState == DNS_WAIT_ANSWER)
      DNSState = DNS_INIT;
#endif
#if 0
    // Check if we have to send to WEB server (and send only if DNS lookup was OK or not needed)
    if (((byte)((byte)Minutes - LastWebSend) >= Config.WebSendPeriod) && DNSState == DNS_GOT_ANSWER) {
      LastWebSend = (byte)Minutes;
      PtData = CommonStrBuff;

      // DataStream 0 is local DS1820 temp
      PtData += sprintf(PtData, "0,");
      if (CentralTempSignBit)
        *PtData++ = '-';
      PtData += sprintf(PtData, "%d.%d\r\n", CentralTempWhole, CentralTempFract);

      // DataStream 1 to ITPLUS_MAX_SENSORS are IT+ Sensors
      for (byte Channel = 0; Channel < ITPLUS_MAX_SENSORS; Channel++) {
        if (ITPlusChannels[Channel].SensorID != 0xff) {  // Send only if registered
          if (ITPlusChannels[Channel].LastReceiveTimer != 0) {  // Send only if valid temp received
            //                        "0,-tt.dNL1,-tt.dNL2,-tt.dNL3,-tt.dNL0"
            PtData += sprintf(PtData, "%d,", Channel + 1);
            if (ITPlusChannels[Channel].Temp & 0x80)
              *PtData++ = '-';
            PtData += sprintf(PtData, "%d.%d\r\n", ITPlusChannels[Channel].Temp & 0x7f, ITPlusChannels[Channel].DeciTemp);
          }
        }
      }

      // DataStream FIRST_JEENODE to FIRST_JEENODE + MAX_JEENODE are RF12 Jeenodes
      for (byte Channel = 0; Channel < MAX_JEENODE; Channel++) {
        if (RF12Channels[Channel].LastReceiveTimer != 0) {  // Send only if valid temp received
          //                        "0,-tt.dNL1,-tt.dNL2,-tt.dNL3,-tt.dNL0"
          PtData += sprintf(PtData, "%d,", Channel + FIRST_JEENODE);
          if (RF12Channels[Channel].Temp & 0x80)
            *PtData++ = '-';
          PtData += sprintf(PtData, "%d.%d\r\n", RF12Channels[Channel].Temp & 0x7F, RF12Channels[Channel].DeciTemp);
        }
      }

      // Remove last CR/LF
      *(PtData - 2) = 0;
#if DEBUG_HTTP
      DebugPrint_P(PSTR("Sending ")); Serial.println(CommonStrBuff);
#endif
      WebSend(CommonStrBuff);
    }
  }

  if (Acquire1820) {
    Get1820Tmp();
    Acquire1820 = false;
  }

  CheckProcessBrowserRequest();
#else
}
#endif

  CheckRF12Recept();

  // Check error condition for signaling through LED
  ErrorCondition = false;
  for (byte Channel = 0; Channel < ITPLUS_MAX_SENSORS; Channel++) {
    if (ITPlusChannels[Channel].LastReceiveTimer == 0) {  // La Crossereceive OK check
      ErrorCondition = true;
      break;
    }
  }
#if 0
  if (!ErrorCondition) {
    for (byte Channel = 0; Channel < MAX_JEENODE; Channel++) {
      if (RF12Channels[Channel].LastReceiveTimer == 0) {  // RF12 receive OK check
        ErrorCondition = true;
        break;
      }
    }
  }
  if (!ErrorCondition) {  // WEB Server send OK check
    if ((byte)((byte)Minutes - (byte)LastServerSendOK) >=  3 * Config.WebSendPeriod)
      ErrorCondition = true;
      // In a next release, trigger also a new DNS lookup in case Server IP changed, for now, assume a reset will do the job.
  }
#endif
  if (ErrorCondition) {
    if (SignalError == 0) SignalError = 1;  // Else, meaning that already signaled
  } else
    SignalError = 0;
}
