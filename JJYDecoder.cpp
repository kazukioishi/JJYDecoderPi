#include "JJYDecoder.h"

#define HIGH 1
#define LOW 0
#define F40KHZ HIGH
#define F60KHZ LOW
//D606C P pin setting to HIGH will power off the module.
#define POWER_ON LOW
#define POWER_OFF HIGH

int JJYDecoder::pinF = 3;
int JJYDecoder::pinTP = 13;
int JJYDecoder::pinP = 2;
JJYDecoder::JJYCODE JJYDecoder::previousCode;
int JJYDecoder::currentPosition = 59;
int JJYDecoder::sync = 0;
struct JJYDecoder::timeCode_t JJYDecoder::timeCode;

JJYDecoder::JJYDecoder(){
    //init
    markerMin = 50;
    markerMax = 350;
    highMin = 350;
    highMax = 650;
    lowMin = 650;
    lowMax = 950;
    DecoderSingleton = *this;
    //init GPIO
    if(wiringPiSetupGpio() == -1){
        throw "GPIO Initialize error.";
        return;
    }
    pinMode(pinF, OUTPUT);
    pinMode(pinTP, INPUT);
    pinMode(pinP, OUTPUT);

    digitalWrite(pinF, F40KHZ);
    digitalWrite(pinP, POWER_ON);

    wiringPiISR(pinTP, INT_EDGE_BOTH, &JJYDecoder::StaticEventCaller);
}

JJYDecoder::~JJYDecoder(){

}

int JJYDecoder::getMinute(long long code) {
 return ((code >> 57) & 0xb111) * 10 + ((code >> 52) & 0xb1111);
}

int JJYDecoder::getHour(long long code) {
 return ((code >> 47) & 0xb1111) * 10 + ((code >> 42) & 0xb1111);
}

int JJYDecoder::getDay(long long code) {
  return ((code >> 37) & 0xb11) * 100 + ((code >> 32) & 0xb1111) * 10 + ((code >> 27) & 0xb1111);
}

int JJYDecoder::getYear(long long code) {
  return ((code >> 16) & 0xb1111) * 10 + ((code >> 12) & 0xb1111);
}

int JJYDecoder::getDayOfWeek(long long code) {
  return ((code >> 8) & 0xb111);
}

int JJYDecoder::getBits(unsigned char value) {
    value = (value & 0x55) + ((value >> 1) & 0x55);
    value = (value & 0x33) + ((value >> 2) & 0x33);
    return (value & 0x0f) + ((value >> 4) & 0x0f);
}

void JJYDecoder::StaticEventCaller(){
    JJYDecoder::DecoderSingleton->intChange();
}

void JJYDecoder::intChange() {
  char buf[128];
  JJYCODE currentCode;
  int interval;
  
  switch (digitalRead(pinTP)) {
    case HIGH:
      timeHigh = clock();
      return;
    case LOW:
      interval = clock() - timeHigh;
      if (markerMin < interval && interval <= markerMax) {   // Marker
        currentCode = JJYCODE_M;
      } else if (highMin < interval && interval <= highMax) {  // HIGH
        currentCode = JJYCODE_H;
      } else if (lowMin < interval && interval <= lowMax) {    // LOW
        currentCode = JJYCODE_L;
      } else {
        return;
      }
      break;
  }
  
  sprintf(buf, "Value = %d, %c", interval, currentCode);
  cout << buf << "\n";
  
  if (sync) {
    switch (currentCode) {
      case JJYCODE_M:
      case JJYCODE_L:
        timeCode.code &= ~(1LL << currentPosition);
        break;
      case JJYCODE_H:
        timeCode.code |= 1LL << currentPosition;
        break;
    }
    
    switch (currentPosition--) {
      // Position Marker
      case 51: case 41: case 31: case 21: case 11: case 1:
        if (currentCode != JJYCODE_M) {
          cout << "Position Marker Error\n";
          sync = 0;
        }
        break;
      // Fixed to 0
      case 56: case 50: case 49: case 46: case 40: case 39: 
      case 36: case 26: case 25: case 5: case 4: case 3: case 2:
        if (currentCode != JJYCODE_L) {
          cout << "Fixed 0 Error";
          sync = 0;
        }
        break;
      // Parity of hour
      case 24:
        if (((getBits(timeCode.code >> 42) & 0xff) % 2) != ((timeCode.code >> 24) & 1)) {
          cout << "Hour Parity Error";
          sync = 0;
        }
        break;
      // Parity of minute
      case 23:
        if (((getBits(timeCode.code >> 52) & 0xff) % 2) != ((timeCode.code >> 23) & 1)) {
          cout << "Minute Parity Error";
          sync = 0;
        }
        break;
      case 0:
        sprintf(buf, "%02d:%02d, %03ddays, %2dyear, %1d Day of wees", 
          getHour(timeCode.code), getMinute(timeCode.code), getDay(timeCode.code), getYear(timeCode.code), getDayOfWeek(timeCode.code));
        cout << buf << "\n";
      
        for (int i = 59; i >= 0; i--) {
          if (timeCode.code & (1LL << i)) {
            cout << "1\n";
          } else {
            cout << "0\n";
          }
        }
        currentPosition = 59;
        break;
    }
  } else {
    if (previousCode == JJYCODE_M && currentCode == JJYCODE_M) {
      sync = 1;
      currentPosition = 59;
    }
  }
  previousCode = currentCode;
}