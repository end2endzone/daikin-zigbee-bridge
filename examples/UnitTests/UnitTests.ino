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
static uint16_t LAST_MESSAGE_BUFFER_SIZE_IN_BYTES = 512;
static Protocol::message_info_t last_message_received = {};


//*******************************************************************************************************************
//  Unit test functions
//*******************************************************************************************************************

void handleMessage(uint8_t msg_id, const uint8_t* payload, uint16_t payload_size) {

  last_message_received.id = msg_id;
  last_message_received.payload_size = payload_size;
  if (payload) {
    #define truncate_size(a,b) (((a) < (b)) ? (a) : (b))
    uint16_t copy_size = truncate_size(payload_size, LAST_MESSAGE_BUFFER_SIZE_IN_BYTES);
    memcpy(last_message_received.payload, payload, copy_size);
  }
}

void resetLastMessageCapture() {
  last_message_received.id = 0;
  memset(last_message_received.payload, 0, LAST_MESSAGE_BUFFER_SIZE_IN_BYTES);
  last_message_received.payload_size = 0;
  last_message_received.crc = 0;
}

bool isLastMessageCaptureEmpty() {
  if (last_message_received.id == 0 &&
      last_message_received.payload_size == 0 &&
      last_message_received.crc == 0 )
    return true;
  return false;
}

bool isEmptyBuffer(uint8_t * buffer, size_t size) {
  if (buffer == nullptr)
    return true;
  if (size == 0)
    return true;
  
  bool isZero = true;
  for(uint16_t i=0; i<size && isZero == true; i++) {
    isZero &= (buffer[i] == 0);
  }
  return isZero;
}

TestResult AssertLastMessageCaptureEmpty() {
  ASSERT_EQ(last_message_received.id, 0);
  ASSERT_EQ(last_message_received.payload_size, 0);
  if (last_message_received.payload != nullptr && last_message_received.payload_size > 0) {
    ASSERT_TRUE(isEmptyBuffer(last_message_received.payload, last_message_received.payload_size));
  }
  ASSERT_EQ(last_message_received.crc, 0);
  return TestResult::Pass;
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

  uint8_t buffer[256];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  Protocol::message_info_s expected_msg = {};
  expected_msg.id = (__LINE__%255);
  expected_msg.payload = (uint8_t*)"I am a Jedi, like my father before me.";
  expected_msg.payload_size = strlen((const char *)expected_msg.payload);
  
  // Send the message
  proto.sendMessage(expected_msg.id, expected_msg.payload, expected_msg.payload_size);

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_EQ(mock.available(), 0);

  // Move the bytes written to the mock from the write buffer to the read buffer.
  // No corruption or missing bytes.
  mock.copyTxToRxBuffer();

  // Ingest the sent message, reading all incomming bytes
  proto.loop();

  // Assert the same message was received.
  ASSERT_EQ(expected_msg.id, last_message_received.id);
  ASSERT_EQ(expected_msg.payload_size, last_message_received.payload_size);
  ASSERT_EQ(memcmp(expected_msg.payload, last_message_received.payload, expected_msg.payload_size), 0);

  return TestResult::Pass;
}

TestResult testInvalidCRC() {
  MockSerial mock;

  uint8_t buffer[256];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  Protocol::message_info_s expected_msg = {};
  expected_msg.id = (__LINE__%255);
  expected_msg.payload = (uint8_t*)"I'm Luke Skywalker. I'm here to rescue you.";
  expected_msg.payload_size = strlen((const char *)expected_msg.payload);
  
  // Send the message
  proto.sendMessage(expected_msg.id, expected_msg.payload, expected_msg.payload_size);

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_EQ(mock.available(), 0);

  // Move the bytes written to the mock from the write buffer to the read buffer.
  mock.copyTxToRxBuffer();

  // Change the content to force a CRC check fail
  ASSERT_TRUE(mock.corruptRxByte(Protocol::SYNC_SIGNATURE_SIZE + 6, '!')); // change a byte somewhere in the payload

  // Ingest the sent message, reading all incomming bytes
  proto.loop();

  // Assert no message was received.
  TestResult result = AssertLastMessageCaptureEmpty();
  ASSERT_TEST_RESULT(result);

  return TestResult::Pass;
}

TestResult testMessageCutOffWithMissingBytes() {
  MockSerial mock;

  uint8_t buffer[256];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  Protocol::message_info_s expected_msg = {};
  expected_msg.id = (__LINE__%255);
  expected_msg.payload = (uint8_t*)"I'm Luke Skywalker. I'm here to rescue you.";
  expected_msg.payload_size = strlen((const char *)expected_msg.payload);
  
  // Send the message
  proto.sendMessage(expected_msg.id, expected_msg.payload, expected_msg.payload_size);

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_EQ(mock.available(), 0);

  // Move the bytes written to the mock from the write buffer to the read buffer.
  mock.copyTxToRxBuffer();

  // Change the content to force a CRC check fail
  ASSERT_TRUE(mock.dropRxByte(Protocol::SYNC_SIGNATURE_SIZE + 6)); // drop a byte somewhere in the payload

  // Ingest the sent message, reading all incomming bytes
  proto.loop();

  // Assert no message was received.
  TestResult result = AssertLastMessageCaptureEmpty();
  ASSERT_TEST_RESULT(result);

  return TestResult::Pass;
}

TestResult testForcedResynchronizationInMiddleOfMessage() {
  MockSerial mock;

  uint8_t buffer[128];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  Protocol::message_info_s expected_msg1 = {};
  expected_msg1.id = (__LINE__%255);
  expected_msg1.payload = (uint8_t*)"I'm Luke Skywalker. I'm here to rescue you.";
  expected_msg1.payload_size = strlen((const char *)expected_msg1.payload);

  Protocol::message_info_s expected_msg2 = {};
  expected_msg2.id = (__LINE__%255);
  expected_msg2.payload = (uint8_t*)"I'll never turn to the dark side.";
  expected_msg2.payload_size = strlen((const char *)expected_msg2.payload);
  
  // Send the message #1
  proto.sendMessage(expected_msg1.id, expected_msg1.payload, expected_msg1.payload_size);

  // Drop 5 bytes from message #1
  size_t target_size = mock.getTxSize() - 5;
  while(mock.getTxSize() > target_size) {
    ASSERT_TRUE(mock.dropLastTxByte());
  }

  // Send the message #2
  proto.sendMessage(expected_msg2.id, expected_msg2.payload, expected_msg2.payload_size);

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_EQ(mock.available(), 0);

  // Move the bytes written to the mock from the write buffer to the read buffer.
  mock.copyTxToRxBuffer();

  // Ingest the sent message, reading all incomming bytes
  proto.loop();

  // Assert message #2 was received.
  ASSERT_EQ(expected_msg2.id, last_message_received.id);
  ASSERT_EQ(expected_msg2.payload_size, last_message_received.payload_size);
  ASSERT_EQ(memcmp(expected_msg2.payload, last_message_received.payload, expected_msg2.payload_size), 0);

  return TestResult::Pass;
}

TestResult testZeroByteEmptyPayload() {
  MockSerial mock;

  uint8_t buffer[256];
  Protocol proto(&mock, buffer, sizeof(buffer));

  proto.onMessageReceived(handleMessage);

  resetLastMessageCapture();

  Protocol::message_info_s expected_msg = {};
  expected_msg.id = (__LINE__%255);
  expected_msg.payload = (uint8_t*)"";
  expected_msg.payload_size = strlen((const char *)expected_msg.payload);
  
  // Send the message
  proto.sendMessage(expected_msg.id, expected_msg.payload, expected_msg.payload_size);

  // Make sure the SerialInterface reports nothing readable to this point
  ASSERT_EQ(mock.available(), 0);

  // Move the bytes written to the mock from the write buffer to the read buffer.
  // No corruption or missing bytes.
  mock.copyTxToRxBuffer();

  // Ingest the sent message, reading all incomming bytes
  proto.loop();

  // Assert the same message was received.
  ASSERT_EQ(expected_msg.id, last_message_received.id);
  ASSERT_EQ(expected_msg.payload_size, last_message_received.payload_size);
  //ASSERT_EQ(memcmp(expected_msg.payload, last_message_received.payload, expected_msg.payload_size), 0);

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
  TEST(testInvalidCRC);
  TEST(testMessageCutOffWithMissingBytes);
  TEST(testForcedResynchronizationInMiddleOfMessage);
  TEST(testZeroByteEmptyPayload);

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
