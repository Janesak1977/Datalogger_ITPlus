/**
 * Temperature data logger.
 *
 * This module is in charge of receiving other Jeenodes measures through RF12
 *
 * Gérard Chevalier, Jan 2011
 * Nov 2011, changed to integrate La Crosse IT+ protocol
 */

#include "DataloggerDefs.h"
#include "RF12WS.h"
//#include <Ports.h>

// After IT+ addition into central node, added 2 preamble bytes to identify clearly the "Home Made Sensor" over RF12
#define CHECK1  0x10
#define CHECK2  0x01


Type_Channel RF12Channels[MAX_JEENODE];

void RF12Init() {
  rf12_initialize(1, RF12_868MHZ, 0xd4);

  // Overide settings for RFM01/IT+ compliance
  rf12_initialize_overide_ITP();

  RF12Channels[0].LastReceiveTimer = 0;
  RF12Channels[1].LastReceiveTimer = 0;
  RF12Channels[2].LastReceiveTimer = 0;
  DebugPrintln_P(PSTR("RF12 Initialized"));
}

void CheckRF12Recept() {
  byte Channel;

  if (rf12_recvDone()) {
    // If a "Receive Done" condition is signaled, we can safely use the RF12 library buffer up to the next call to
    // rf12_recvDone: RF12 tranceiver is held in idle state up to the next call.
    // Is it IT+ or Jeenode frame ?
    if (ITPlusFrame)
      ProcessITPlusFrame();  // Keep IT+ logic outside this source files
    else {
      if (rf12_crc == 0) {  // Valid RF12 Jeenode frame received
#ifdef RF12_DEBUG
        DebugPrint_P(PSTR("RF12 rcv: "));
        for (byte i = 0; i < rf12_len; ++i) {
          Serial.print(rf12_data[i], HEX); Serial.print(' ');
        }
        Serial.println();
#endif
        // RF12 frame format: Byte 0 & 1 = preamble to identify clearly the "Home Made Sensor"
        // Byte 2 = channel, Byte 3 = temp (bit 7 == sign), Byte 4 = decitemp
        if (rf12_data[0] == CHECK1 && rf12_data[1] == CHECK2) {
          Channel = rf12_data[2];
          if (Channel >= 1 && Channel <= MAX_JEENODE) {
            RF12Channels[Channel - 1].Temp = rf12_data[3] & 0b01111111;
              if ((rf12_data[3] & 0x80) != 0) RF12Channels[Channel - 1].Temp |= 0x80;
              RF12Channels[Channel - 1].DeciTemp = rf12_data[4];
              RF12Channels[Channel - 1].LastReceiveTimer = SENSORS_RX_TIMEOUT;
          }
          if (SignalError == 0) {  // Otherwise don't touch LED
            RX_LED_ON(); delay(300);
            RX_LED_OFF(); delay(200);
            RX_LED_ON(); delay(300);
            RX_LED_OFF();
          }
        }
      }
    }
  }
}


