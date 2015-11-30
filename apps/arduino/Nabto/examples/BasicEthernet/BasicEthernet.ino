/**
 * Nabto - simple and secure - www.nabto.com
 *
 * This is an example uNabto application on the Arduino platform.
 * This basic example demonstrates toggling an LED over the internet,
 * but is easily extended to your needs.
 *
 * Hardware required:
 * - Arduino Uno or Duemilanove
 * - Arduino Ethernet shield
 * - Standard LED
 *
 * How-To:
 * - Edit the mac variable to match the MAC-address located on the bottom of the Ethernet shield.
 * - Specify an unique ID for your Arduino demo.
 * - Connect the ethernet shield to the Arduino.
 * - Connect the LED to pin A0 (anode) and ground (cathode).
 * - Upload and run.
 * - Type in the unique id in your browser.
 *
 * The onboard LED on digital pin 13 is not to your disposal, 
 * since it is used by the ethernet shield.
 */

#include <Nabto.h>


// Use mac address from the bottom of the ethernet shield
byte mac[] = { 0x90, 0xa2, 0xda, 0x00, 0x3c, 0x60 };

// Choose an id for this uNabto device ending with <.sdk.u.nabto.net>
char* id = (char*)"demo.sdk.u.nabto.net";

// Pin to toggle LED
int ledPin = A0;

void setup() {
  // Set LED pin to output
  pinMode(ledPin, OUTPUT);

  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize Nabto
  Serial.println("Init...");
  Nabto.begin(mac, id);

  // Optionally get nabto firmware version
  char versionString[10];
  Nabto.version(versionString);

  Serial.print("Nabto v");
  Serial.print(versionString);
  Serial.print(" running...");
}

void loop() {
  // Check for new nabto udp packets and send response. Non-blocking
  Nabto.tick();

  // We have chosen to sleep 10 milliseconds between tics
  delay(10);
}

// Set first onboard LED and return state,
// only using ID #1 in this simple example  
uint8_t setLed(char led_id, char led_on) {
  if (led_id == 1) {
    digitalWrite(ledPin, led_on);
    return digitalRead(ledPin);
  }
  else {
    return 0;
  }
}

// Return first onboard LED state,
// only using ID #1 in this simple example
uint8_t readLed(char led_id) {
  if (led_id == 1) {
    return digitalRead(ledPin);
  }
  else {
    return 0;
  }
}

/***************** The uNabto application logic *****************
 * This is where the user implements his/her own functionality
 * to the device. When a Nabto message is received, this function
 * gets called with the message's request id and parameters.
 * Afterwards a user defined message can be sent back to the
 * requesting browser.
 ****************************************************************/
application_event_result application_event(application_request* request, buffer_read_t* read_buffer, buffer_write_t* write_buffer) {
  switch(request->queryId) {
  case 1: 
    {
      //  <query name="light_write.json" description="Turn light on and off" id="1">
      //    <request>
      //      <parameter name="light_id" type="uint8"/>
      //      <parameter name="light_on" type="uint8"/>
      //    </request>
      //    <response>
      //      <parameter name="light_state" type="uint8"/>
      //    </response>
      //  </query>

      uint8_t light_id;
      uint8_t light_on;
      uint8_t light_state;

      // Read parameters in request
      if (!buffer_read_uint8(read_buffer, &light_id)) return AER_REQ_TOO_SMALL;
      if (!buffer_read_uint8(read_buffer, &light_on)) return AER_REQ_TOO_SMALL;

      // Set light according to request
      light_state = setLed(light_id, light_on);

      // Write back led state
      if (!buffer_write_uint8(write_buffer, light_state)) return AER_REQ_RSP_TOO_LARGE;

      return AER_REQ_RESPONSE_READY;
    }
  case 2: 
    {
      //  <query name="light_read.json" description="Read light status" id="2">
      //    <request>
      //      <parameter name="light_id" type="uint8"/>
      //    </request>
      //    <response>
      //      <parameter name="light_state" type="uint8"/>
      //    </response>
      //  </query>

      uint8_t light_id;
      uint8_t light_state;

      // Read parameters in request
      if (!buffer_read_uint8(read_buffer, &light_id)) return AER_REQ_TOO_SMALL;

      // Read light state
      light_state = readLed(light_id);

      // Write back led state
      if (!buffer_write_uint8(write_buffer, light_state)) return AER_REQ_RSP_TOO_LARGE;

      return AER_REQ_RESPONSE_READY;
      
    default:
      return AER_REQ_INV_QUERY_ID;
    }
  }
}
