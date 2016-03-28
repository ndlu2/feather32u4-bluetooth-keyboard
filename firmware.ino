#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Keyboard.h"
#include "BluefruitConfig.h"
#define DEBUG 1

#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"



/* Bluefruit object with hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

uint8_t values[ROWS][COLS] = {
  {KEY_A, KEY_B},
  {KEY_C, KEY_D}
};
//output pins
uint8_t row_pins[ROWS] = {5, 6};

//input pins
uint8_t col_pins[COLS] = {10, 11};


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{

  Serial.begin(115200);
  
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  
  //Set high
  
  #ifdef FOO
 /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );



  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'Bluefruit Keyboard': "));
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Not a Keyboard" )) ) {
    error(F("Could not set device name?"));
  }

  /* Enable HID Service */
  Serial.println(F("Enable HID Service (including Keyboard): "));
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    if ( !ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ))) {
      error(F("Could not enable Keyboard"));
    }
  }else
  {
    if (! ble.sendCommandCheckOK(F( "AT+BleKeyboardEn=On"  ))) {
      error(F("Could not enable Keyboard"));
    }
  }

  /* Add or remove service requires a reset */
  Serial.println(F("Performing a SW reset (service changes require a reset): "));
  if (! ble.reset() ) {
    error(F("Couldn't reset??"));
  }

#endif
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  uint8_t key_count, r, c, value, modifier, count;
  
  uint8_t debounce[COLS];
  uint8_t keys[MAX_KEYS];
  
  
  key_count = 0;
  modifier = 0;
  for(r = 0; r < ROWS; r++)
  {
    digitalWrite(row_pins[r], 1);
    
    //reset debounce counter
    for( c = 0; c < COLS; c++)
    {
        debounce[c] = 0;
    }
    //#TODO implement n sequential
    for(count = 0; count < DEBOUNCE; count++)
    {
      delay(DEBOUNCE_DELAY);
      for(c = 0; c < COLS; c++)
      {
        debounce[c] += !digitalRead(col_pins[c]);
      }
    }
    digitalWrite(row_pins[r], 0);
    for(c = 0; c < COLS; c++)
    {
      if(debounce[c] > DEBOUNCE_MIN)
      {
        value = values[r][c];
        
        //if the value is a modifier
        if(key_count < MAX_KEYS)
        {
          keys[key_count] = value;
          key_count ++;
        }
      }
    }
  }
  ble.print(F("AT+BLEKEYBOARDCODE="));
  ble.print(modifier, HEX);
  ble.print(F("-00"));
  for(count = 0; count < key_count; count++)
  {
    ble.print(F("-"));
    ble.print(keys[count], HEX);
  }
  ble.println();
  
  if( !ble.waitForOK() )
  {
    Serial.println( F("FAILED!") );
  }
}
