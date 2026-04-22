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
static uint8_t lastMsgId = 0;
static uint8_t lastPayload[512];
static uint16_t lastPayloadLen = 0;

//*******************************************************************************************************************
//  Unit test functions
//*******************************************************************************************************************

void handleMessage(uint8_t msgId, const uint8_t* payload, uint16_t payloadLen) {
  log_i("handleMessage(): msgId=%d, payload=0x%04X, payloadLen=%d", msgId, payload, payloadLen);

  lastMsgId = msgId;
  lastPayloadLen = payloadLen;
  if (payload) {
    #define truncate_size(a,b) (((a) < (b)) ? (a) : (b))
    uint16_t copy_size = truncate_size(payloadLen, sizeof(lastPayload));
    log_i("Copying %u bytes from payload (0x%04X) to lastPayload (0x%04X) which is %u bytes.", copy_size, payload, lastPayload, sizeof(lastPayload));
    memcpy(lastPayload, payload, copy_size);
  }
}

//void sendThroughMock(MockSerial &mock, Protocol &proto, uint8_t msgId, const uint8_t* payload, uint16_t payloadLen) {
//  mock.txBuffer.clear();
//  proto.sendMessage(msgId, payload, payloadL en);
//  mock.pushRx(mock.txBuffer.data(), mock.txBuffer.size());
//}

void resetLastMessageCapture() {
  lastMsgId = 0;
  lastPayloadLen = 0;
  memset(lastPayload, 0, sizeof(lastPayload));
}

void resetTestData()
{
  resetLastMessageCapture();
}

const char *getMillisTimestamp()
{
  static char gTimestampBuffer[16];
  sprintf(gTimestampBuffer, "%06d: ", millis());
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

  uint8_t buffer[512];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  const uint8_t expectedMsgId = 3;
  const uint8_t expectedPayload[] = {0x10, 0x20, 0x30};
  const uint16_t expectedPayloadSize = sizeof(expectedPayload);
  
  // Send the message
  proto.sendMessage(expectedMsgId, expectedPayload, expectedPayloadSize);

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
  ASSERT_EQ(expectedMsgId, lastMsgId);
  ASSERT_EQ(expectedPayloadSize, lastPayloadLen);
  ASSERT_EQ(memcmp(expectedPayload, lastPayload, expectedPayloadSize), 0);
}

//*******************************************************************************************************************
//  Sketch logic
//*******************************************************************************************************************

void setup()
{
  Serial.begin(115200);
  //Serial1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);

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
