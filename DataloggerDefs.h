/**
 * Temperature data logger.
 *
 * Global Includes
 *
 * Gérard Chevalier, Jan 2011
 * Nov 2011, changed to integrate La Crosse IT+ protocol
 */
#include "Arduino.h"

// Central Node DS1820 sensor debug flags
//#define DS1820_DEBUG
//#define DS1820_DEBUG_LOW

// IT+ Decoding debug flags
#define ITPLUS_DEBUG_FRAME
#define ITPLUS_DEBUG
#define RINIE

// Network debuging flag
//#define DEBUG_ETH 1 // set to 1 to show incoming requests on serial port
#define DEBUG_DNS 1
//#define DEBUG_HTTP 1

// RF12 transmissions debuging flag
#define RF12_DEBUG

#ifdef DS1820_DEBUG_LOW
  #define DS1820_DEBUG
#endif

#define SENSORS_RX_TIMEOUT 5

#ifndef DATALOGGERDEFS

#define ITPLUS_MAX_SENSORS  5
#define ITPLUS_MAX_DISCOVER  5
#define ITPLUS_DISCOVERY_PERIOD 255

#define FIRST_JEENODE  10
#define MAX_JEENODE    3

// Structure in RAM holding configuration, as stored in EEPROM
typedef struct {
    byte LocalIP[4];    // Keep those 3 parameters at the begining
    byte RouterIP[4];   // for the hard reset config
    byte ServerIP[4];   // working
    byte WebSendPeriod;
    byte ITPlusID[ITPLUS_MAX_SENSORS];  // IT+ Sensors ID for Registered sensors
} Type_Config;

#define DNS_INIT	0
#define DNS_WAIT_ANSWER	1
#define DNS_GOT_ANSWER	2
#define DNS_NO_HOST	3

// Radio Sensor structure (both RF12 & IT+)
typedef struct {
  byte SensorID;
  byte LastReceiveTimer;
  byte Temp, DeciTemp;
} Type_Channel;

// Radio Sensor structure for IT+ Sensors discovery process
typedef struct {
  byte SensorID;
  byte LastReceiveTimer;
  byte Temp, DeciTemp;
} Type_Discovered;

#define RX_LED_ON()   ((PORTC |=  (1<<PORTC3)))
#define RX_LED_OFF()  ((PORTC &= ~(1<<PORTC3)))

#define PARAM_RESET_DATA_PORT  PORTD
#define PARAM_RESET_PIN_PORT   PIND
#define PARAM_RESET_PIN_NB     6
extern void DebugPrint_P(const char *);
extern void DebugPrintln_P(const char *);
extern void ProcessITPlusFrame();

extern void rf12_initialize_overide_ITP ();
extern boolean ITPlusFrame;

#define DATALOGGERDEFS
#endif

