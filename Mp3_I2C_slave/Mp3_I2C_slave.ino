#include <Wire.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

SoftwareSerial mySerial(2, 3); // RX, TX
#define MAX_BUFFER 10
int last = 0; 
int buffer[MAX_BUFFER];
char volume = 30;

void setup() {
  Wire.begin(8);  /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  mySerial.begin(9600);
  mp3_set_serial(mySerial);

  pinMode(4, INPUT);
  Serial.print("Arduino ready.");
}

void loop() {
  if(last > 0 && digitalRead(4) == 1) {
      int value = buffer[last-- - 1];
      Serial.print("Arduino: playing #"); Serial.println(value);
      mp3_set_volume(volume);
      mp3_play (value);
      delay(1000);
  }
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  while (Wire.available() >= 4) {
    if (Wire.read() == 0xFF) {
      volume = Wire.read();
      char size = Wire.read();
      for(int i = 0;i < size;i++) {
      char low = Wire.read();
      char high = Wire.read();

      const int value = ((high & 0x7f) << 7) | low;
      Serial.print("Arduino: received event ");Serial.println(value);
      buffer[last++] = value;
      }
    }
  }
}
