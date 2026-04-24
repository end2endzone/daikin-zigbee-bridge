#include <Arduino.h>

#include <stdint.h>
#include <sstream>

#include "Protocol.h"
#include "MockSerial.h"

#include "TestingFramework.hpp"
#include "LoggingFramework.hpp"
#include "StringFormatter.hpp"

// project's constants & variables
//#define UART1_TX_PIN 17
//#define UART1_RX_PIN 16

// HardwareSerialAdapter serialAdapter(Serial1);
//MockSerial mockSerial;

// Allocate buffer in sketch
//static uint8_t protoBuffer[1024];

//Protocol proto(mockSerial, protoBuffer, sizeof(protoBuffer));

// Global variables to capture callback results
static uint16_t LAST_MESSAGE_BUFFER_SIZE_IN_BYTES = 128;
static Protocol::message_info_t last_message_received = {};

//*******************************************************************************************************************
//  Unit test functions
//*******************************************************************************************************************

void handleMessage(uint8_t msg_id, const uint8_t* payload, uint16_t payload_size) {
  log_i("handleMessage(): msg_id=%d, payload=0x%04X, payload_size=%d", msg_id, payload, payload_size);

  last_message_received.id = msg_id;
  last_message_received.payload_size = payload_size;
  if (payload) {
    #define truncate_size(a,b) (((a) < (b)) ? (a) : (b))
    uint16_t copy_size = truncate_size(payload_size, LAST_MESSAGE_BUFFER_SIZE_IN_BYTES);
    log_i("Copying %u bytes from payload (0x%04X) to last_message_received.payload (0x%04X) which is %u bytes.", copy_size, payload, last_message_received.payload, LAST_MESSAGE_BUFFER_SIZE_IN_BYTES);
    
    memcpy(last_message_received.payload, payload, copy_size);
    //for(uint16_t i=0; i<copy_size; i++) {
    //  last_message_received.payload[i] = payload[i];
    //}
  }
}

void resetLastMessageCapture() {
  last_message_received.id = 0;
  last_message_received.payload_size = 0;
  memset(last_message_received.payload, 0, LAST_MESSAGE_BUFFER_SIZE_IN_BYTES);
}

void resetTestData()
{
  resetLastMessageCapture();
}

const char *getMillisTimestamp()
{
  static char gTimestampBuffer[16];
  sprintf(gTimestampBuffer, "%06lu: ", millis());
  return gTimestampBuffer;
}

const char *getBoardDescriptor()
{
#if defined(ARDUINO_AVR_UNO)
  return "Arduino Uno";
#elif defined(ARDUINO_AVR_NANO)
  return "Arduino Nano";
#elif defined(ARDUINO_AVR_MEGA2560)
  return "Arduino Mega 2560";
#elif defined(ESP8266)
  return "ESP8266";
#elif defined(ESP32)
  return "ESP32";
#else
  return "Unknown Board";
#endif
}

//*******************************************************************************************************************
//  Tests
//*******************************************************************************************************************

TestResult testThisTestAlwaysPass()
{
  return TestResult::Pass;
}

TestResult testThisTestAlwaysFailsStringContains()
{
  ASSERT_STRING_CONTAINS("foo", "this is a complete haystack string");
  return TestResult::Pass;
}

TestResult testThisTestAlwaysFailsIntegerEquals()
{
  ASSERT_EQ(-1, 42);
  return TestResult::Pass;
}

TestResult testNormalMessage() {
  MockSerial mock;

  uint8_t buffer[128];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  const uint8_t expected_msg_id = 125;
  const uint8_t expected_payload[] = {0x10, 0x20, 0x30};
  const uint16_t expected_payload_size = sizeof(expected_payload);
  
  // Send the message
  proto.sendMessage(expected_msg_id, expected_payload, expected_payload_size);

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_EQ(mock.available(), 0);

  // Move the bytes written to the mock from the write buffer to the read buffer.
  // No corruption or missing bytes.
  mock.rxBuffer = mock.txBuffer;

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_NE(mock.available(), 0);

  // Ingest the sent message, reading all incomming bytes
  proto.loop();

  // Assert the same message was received.
  ASSERT_EQ(expected_msg_id, last_message_received.id);
  ASSERT_EQ(expected_payload_size, last_message_received.payload_size);
  ASSERT_EQ(memcmp(expected_payload, last_message_received.payload, expected_payload_size), 0);

  return TestResult::Pass;
}

//*******************************************************************************************************************
//  Sketch logic
//*******************************************************************************************************************

void setup()
{
  Serial.begin(115200);
  //Serial1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);

  // Allocate buffer for capturing lastest message read by Protocol class.
  last_message_received.payload = new uint8_t[LAST_MESSAGE_BUFFER_SIZE_IN_BYTES];
  last_message_received.payload_size = LAST_MESSAGE_BUFFER_SIZE_IN_BYTES;

  Serial.println();

  Serial.println("ESP32 Ready");

  // Print a TEST HEADER
  unsigned long testStartMs = millis();
  LOG("Running tests on a %s.\n", getBoardDescriptor());
  LOG("Current time is %d.\n", testStartMs);
  LOG("--------------------------------------------------------\n");

  //TEST(testThisTestAlwaysPass);
  //TEST(testThisTestAlwaysFailsStringContains);
  //TEST(testThisTestAlwaysFailsIntegerEquals);

  TEST(testNormalMessage);

  // Print a TEST FOOTER
  unsigned long testEndMs = millis();
  unsigned long elapsedMs = testEndMs - testStartMs;
  LOG("--------------------------------------------------------\n");
  LOG("All test completed.\n");
  LOG("Elapsed time in milliseconds: %d\n", elapsedMs);
}

void loop()
{
}
