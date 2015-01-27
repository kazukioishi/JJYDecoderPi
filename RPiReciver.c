#include <stdio.h>

#define HIGH 1
#define LOW 0
#define F40KHZ HIGH
#define F60KHZ LOW
#define POWER_ON LOW
#define POWER_OFF HIGH
long markerMin = 50, markerMax = 350;
long highMin = 350, highMax = 650;
long lowMin = 650, lowMax = 950;

struct timeCode_t {
  unsigned long long code:60;
};

static int pinF = 3;
static int pinTP = 13;
static int pinP = 2;

volatile long timeHigh = 0;
volatile long timeLow = 0;

int getMinute(long long code) {
 return ((code >> 57) & 0b111) * 10 + ((code >> 52) & 0b1111);
}

int getHour(long long code) {
 return ((code >> 47) & 0b1111) * 10 + ((code >> 42) & 0b1111);
}

int getDay(long long code) {
  return ((code >> 37) & 0b11) * 100 + ((code >> 32) & 0b1111) * 10 + ((code >> 27) & 0b1111);
}

int getYear(long long code) {
  return ((code >> 16) & 0b1111) * 10 + ((code >> 12) & 0b1111);
}

int getDayOfWeek(long long code) {
  return ((code >> 8) & 0b111);
}

int getBits(unsigned char value) {
    value = (value & 0x55) + ((value >> 1) & 0x55);
    value = (value & 0x33) + ((value >> 2) & 0x33);
    return (value & 0x0f) + ((value >> 4) & 0x0f);
}

void intChange () {
  char buf[128];
  static char previousCode = '\0';
  char currentCode = '-';
  static int currentPosition = 59;
  static int sync = 0;
  static struct timeCode_t timeCode;
  int interval;
  
  switch (digitalRead(pinTP)) {
    case HIGH:
      timeHigh = millis();
      return;
    case LOW:
      interval = millis() - timeHigh;
      if (markerMin < interval && interval <= markerMax) {   // Marker
        currentCode = 'M';
      } else if (highMin < interval && interval <= highMax) {  // HIGH
        currentCode = 'H';
      } else if (lowMin < interval && interval <= lowMax) {    // LOW
        currentCode = 'L';
      } else {
        return;
      }
      break;
  }
  
  sprintf(buf, "Value = %d, %c", interval, currentCode);
  Serial.println(buf);
  
  if (sync) {
    switch (currentCode) {
      case 'M':
      case 'L':
        timeCode.code &= ~(1LL << currentPosition);
        break;
      case 'H':
        timeCode.code |= 1LL << currentPosition;
        break;
    }
    
    switch (currentPosition--) {
      // Position Marker
      case 51: case 41: case 31: case 21: case 11: case 1:
        if (currentCode != 'M') {
          Serial.println("Position Marker Error");
          sync = 0;
        }
        break;
      // Fixed to 0
      case 56: case 50: case 49: case 46: case 40: case 39: 
      case 36: case 26: case 25: case 5: case 4: case 3: case 2:
        if (currentCode != 'L') {
          Serial.println("Fixed 0 Error");
          sync = 0;
        }
        break;
      // Parity of hour
      case 24:
        if (((getBits(timeCode.code >> 42) & 0xff) % 2) != ((timeCode.code >> 24) & 1)) {
          Serial.println("Hour Parity Error");
          sync = 0;
        }
        break;
      // Parity of minute
      case 23:
        if (((getBits(timeCode.code >> 52) & 0xff) % 2) != ((timeCode.code >> 23) & 1)) {
          Serial.println("Minute Parity Error");
          sync = 0;
        }
        break;
      case 0:
        sprintf(buf, "%02d:%02d, %03ddays, %2dyear, %1d Day of wees", 
          getHour(timeCode.code), getMinute(timeCode.code), getDay(timeCode.code), getYear(timeCode.code), getDayOfWeek(timeCode.code));
        Serial.println(buf);
      
        for (int i = 59; i >= 0; i--) {
          if (timeCode.code & (1LL << i)) {
            Serial.print("1");
          } else {
            Serial.print("0");
          }
        }
        Serial.println("");
        currentPosition = 59;
        break;

    }
    
  } else {
    if (previousCode == 'M' && currentCode == 'M') {
      sync = 1;
      currentPosition = 59;
    }
  }
  previousCode = currentCode;
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  pinMode(pinF, OUTPUT);
  pinMode(pinTP, INPUT);
  pinMode(pinP, OUTPUT);

  digitalWrite(pinF, F40KHZ);
  digitalWrite(pinP, POWER_ON);

  attachInterrupt(pinTP, intChange, CHANGE);
}