#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <printf.h>

#include <SPI.h>


bool radioNumber = 0;
bool role = 0;
byte addresses[][6] = {"1Node", "2Node"};

RF24 radio(7, 8);

void roleZeroProto();
void roleOneProto();

void setup() {
  Serial.begin(115200);
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  if (radioNumber) {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
  } else {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }

  radio.startListening();
}

void loop() {
  if (role == 1) {
    roleOneProto();
  }
  if (role == 0) {
    roleZeroProto();
  }

  if ( Serial.available() ){
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ) {
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)

    } else if ( c == 'R' && role == 1 ) {
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      role = 0;                // Become the primary receiver (pong back)
      radio.startListening();

    }
  }
}

void roleOneProto() {
  radio.stopListening();
  Serial.println(F("Now sending"));
  unsigned long startTime = micros();
  if (!radio.write(&startTime, sizeof(unsigned long))) {
    Serial.println(F("Failed"));
  }

  radio.startListening();
  unsigned long startedWaitingAt = micros();
  boolean timeout = false;

  while (!radio.available()) {
    if (micros() - startedWaitingAt > 200000) {
      timeout = true;
      break;
    }
  }

  if (timeout) {
    Serial.println(F("Failed, response timed out."));
  } else {
    unsigned long gotTime;
    radio.read( &gotTime, sizeof(unsigned long));
    unsigned long endTime = micros();

    Serial.print(F("Sent "));
    Serial.print(startTime);
    Serial.print(F(", Got response "));
    Serial.print(gotTime);
    Serial.print(F(", Round-trip delay "));
    Serial.print(endTime - startTime);
    Serial.println(F(" microseconds"));

    delay(1000);
  }
}

void roleZeroProto() {
  unsigned long gotTime;
  if (radio.available()) {
    while (radio.available()) {
      radio.read(&gotTime, sizeof(unsigned long));
    }

    radio.stopListening();
    radio.write(&gotTime, sizeof(unsigned long));
    radio.startListening();
    Serial.print(F("Sent response "));
    Serial.println(gotTime);
  }
}










