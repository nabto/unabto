/**
 * Nabto - simple and secure - www.nabto.com
 *
 * This is an example uNabto application on the Arduino platform.
 * This basic example demonstrates switching some relays over the internet,
 * but is easily extended to your needs.
 *
 * Hardware required:
 * - Arduino Uno or Ethernet
 * - Arduino Ethernet shield

 *
 * How-To:
 * - Edit the mac variable to match the MAC-address located on the bottom of the Ethernet shield.
 * - Specify an unique ID for your Arduino demo.
 * - Connect the Ethernet Shield to the Arduino if you are using Uno.
 * - Connect the Relay Shield V2.1
 * - Upload and run.
 * - Type in the unique id in your browser.
 *
 */

#include <Nabto.h>


// Use mac address from the bottom of the ethernet shield
byte mac[] = { 0x90, 0xa2, 0xda, 0x00, 0x3c, 0x60 };

// Choose an id for this uNabto device ending with <.relay.sdk.u.nabto.net>
char* id = (char*)"demo.relay.sdk.u.nabto.net";

// Pin to switching the relays
int relay1Pin = 2;
int relay2Pin = 7;
int relay3Pin = 8;
//int relay4Pin = 10; // Ethernet is using this pin!

void setup() {
  // Set relays pin to output
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  //pinMode(relay4Pin, OUTPUT);
  
  //test
  digitalWrite(relay2Pin, HIGH);

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

// Set relay and return state,
// relay 4 is not used in this example.
uint8_t setRelay(char relay_id, char relay_on) {
  switch(relay_id) {
    case 1:
    digitalWrite(relay1Pin, relay_on);
    return digitalRead(relay1Pin);
    break;
    
    case 2:
    digitalWrite(relay2Pin, relay_on);
    return digitalRead(relay2Pin);
    break;
    
    case 3:
    digitalWrite(relay3Pin, relay_on);
    return digitalRead(relay3Pin);
    break;
    
    case 4:
    //digitalWrite(relay4Pin, relay_on);
    //return digitalRead(relay4Pin);
    break;
    
    default:
    return 0;
  }
}

// Return relay state,
// relay 4 is not used in this example.
uint8_t readRelay(char relay_id) {
  switch(relay_id) {
    case 1:
    return digitalRead(relay1Pin);
    break;
    
    case 2:
    return digitalRead(relay2Pin);
    break;
    
    case 3:
    return digitalRead(relay3Pin);
    break;
    
    case 4:
    //return digitalRead(relay4Pin);
    break;
    
    default:
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
application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
  switch(request->queryId) {
  case 1: 
    {
    //  <query name="relay_write.json" description="Turn relay on and off" id="1">
    //    <request>
    //      <parameter name="relay_id" type="uint8"/>
    //      <parameter name="relay_on" type="uint8"/>
    //    </request>
    //    <response>
    //      <parameter name="relay_state" type="uint8"/>
    //    </response>
    //  </query>

      uint8_t relay_id;
      uint8_t relay_on;
      uint8_t relay_state;

      // Read parameters in request
      if (!unabto_query_read_uint8(read_buffer, &relay_id)) return AER_REQ_TOO_SMALL;
      if (!unabto_query_read_uint8(read_buffer, &relay_on)) return AER_REQ_TOO_SMALL;

      // Set relay according to request
      relay_state = setRelay(relay_id, relay_on);
      Serial.print("iAmWriting");
      // Write back relay state
      if (!unabto_query_write_uint8(write_buffer, relay_state)) return AER_REQ_RSP_TOO_LARGE;

      return AER_REQ_RESPONSE_READY;
    }
  case 2: 
    {
    // <query name="relay_read.json" description="Read relay status" id="2">
    //    <request>
    //      <parameter name="relay_id" type="uint8"/>
    //    </request>
    //    <response>
    //      <parameter name="relay_state" type="uint8"/>
    //    </response>
    //  </query>


      uint8_t relay_id;
      uint8_t relay_state;

      // Read parameters in request
      if (!unabto_query_read_uint8(read_buffer, &relay_id)) return AER_REQ_TOO_SMALL;

      // Read relay state
      relay_state = readRelay(relay_id);
      Serial.print(relay_id);
      Serial.print("iAmReading");
      // Write back relay state
      if (!unabto_query_write_uint8(write_buffer, relay_state)) return AER_REQ_RSP_TOO_LARGE;

      return AER_REQ_RESPONSE_READY;
      
    default:
      return AER_REQ_INV_QUERY_ID;
    }
  }
}
