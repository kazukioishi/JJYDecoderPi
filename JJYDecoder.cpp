#include "JJYDecoder.h"

#define HIGH 1
#define LOW 0
#define F40KHZ HIGH
#define F60KHZ LOW
//D606C P pin setting to HIGH will power off the module.
#define POWER_ON LOW
#define POWER_OFF HIGH

int JJYDecoder::pinF = 17;
int JJYDecoder::pinTP = 27;
int JJYDecoder::pinP = 22;
JJYDecoder::JJYCODE JJYDecoder::previousCode;
JJYDecoder *DecoderSingleton;
int JJYDecoder::currentPosition = 59;
bool JJYDecoder::sync = false;
struct JJYDecoder::timeCode_t JJYDecoder::timeCode;
int JJYDecoder::LastHighLow = 3;

JJYDecoder::JJYDecoder(){
    //init
    //marker 200msec
    markerMin = 100;
    markerMax = 350;
    //high 500msec
    highMin = 400;
    highMax = 610;
    //low 800msec
    lowMin = 700;
    lowMax = 910;
    DecoderSingleton = this;
    //init GPIO
    if(wiringPiSetupGpio() == -1){
        throw "GPIO Initialize error.";
        return;
    }
    pinMode(pinF, OUTPUT);
    pinMode(pinTP, INPUT);
    //pullUpDnControl(pinTP, PUD_DOWN);
    pinMode(pinP, OUTPUT);
    pinMode(4, OUTPUT);

    digitalWrite(pinF, F40KHZ);
    digitalWrite(pinP, POWER_ON);

    wiringPiISR(pinTP, INT_EDGE_BOTH, &JJYDecoder::StaticEventCaller);
    cout << "Start OK\n";
}

JJYDecoder::~JJYDecoder(){

}

int JJYDecoder::getMinute(long long code) {
 return ((code >> 57)) * 10 + ((code >> 52) & 0B1111);
}

int JJYDecoder::getHour(long long code) {
 return ((code >> 47) & 0B1111) * 10 + ((code >> 42) & 0B1111);
}

int JJYDecoder::getDay(long long code) {
  return (((code >> 37) & 0B11) * 100 + ((code >> 32) & 0B1111) * 10 + ((code >> 27) & 0B1111));
}

int JJYDecoder::getYear(long long code) {
  return (((code >> 16) & 0B1111) * 10 + ((code >> 12) & 0B1111));
}

int JJYDecoder::getDayOfWeek(long long code) {
  return ((code >> 8) & 0B111);
}

int JJYDecoder::getBits(unsigned char value) {
    value = (value & 0x55) + ((value >> 1) & 0x55);
    value = (value & 0x33) + ((value >> 2) & 0x33);
    return (value & 0x0f) + ((value >> 4) & 0x0f);
}

void JJYDecoder::StaticEventCaller(){
    if(DecoderSingleton != NULL){
        DecoderSingleton->intChange();
    }
}

char* JJYDecoder::getS(JJYCODE c){
  if(c == JJYCODE_M){
     return "M";
  }else if(c == JJYCODE_H){
     return "H";
  }else if(c == JJYCODE_L){
     return "L";
  }
  return "";
}

void JJYDecoder::intChange() {
  //cout << "Change Ivent.\n";
  char buf[128];
  JJYCODE currentCode;
  int interval;
  
  switch (digitalRead(pinTP)) {
    case HIGH:
      digitalWrite(4,HIGH);
      if(LastHighLow == HIGH) return;
      timeHigh = millis();
      LastHighLow = HIGH;
      return;
    case LOW:
      digitalWrite(4,LOW);
      if(LastHighLow == LOW) return;
      interval = millis() - timeHigh;
      //cout << "LOW\n";
      //cout << "interval:" << interval << "msec.\n";
      LastHighLow = LOW;
      if (markerMin < interval && interval <= markerMax) {   // Marker
        currentCode = JJYCODE_M;
      } else if (highMin < interval && interval <= highMax) {  // HIGH
        currentCode = JJYCODE_H;
      } else if (lowMin < interval && interval <= lowMax) {    // LOW
        currentCode = JJYCODE_L;
      } else {
        cout << "Unknown code:" << interval << "msec\n";
        return;
      }
      break;
  }
  
  //sprintf(buf, "Value = %d, %c", interval, getS(currentCode));
  cout << "Value=" << interval << "," << getS(currentCode) << "\n";
  
  if (sync) {
    switch (currentCode) {
      case JJYCODE_M:
        timeCode.code = timeCode.code << 1;
        timeCode.code += 0;
      case JJYCODE_L:
        timeCode.code = timeCode.code << 1;
        timeCode.code += 0;
        break;
      case JJYCODE_H:
        timeCode.code = timeCode.code << 1;
        timeCode.code += 1;
        break;
    }

    currentPosition--;//-1

    switch (currentPosition) {
      // Position Marker
      case 51: case 41: case 31: case 21: case 11: case 1:
        if (currentCode != JJYCODE_M) {
          cout << "Position Marker Error\n";
          sync = false;
        }
        break;
      // Fixed to 0
      case 56: case 50: case 49: case 46: case 40: case 39: 
      case 36: case 26: case 25: case 5: case 4: case 3: case 2:
        if (currentCode != JJYCODE_L) {
          cout << "Fixed 0 Error\n";
          sync = false;
        }
        break;
      // Parity of hour
      /*case 24:
        if (((getBits(timeCode.code >> 42) & 0xff) % 2) != ((timeCode.code >> 24) & 1)) {
          cout << "Hour Parity Error\n";
          sync = false;
        }
        break;*/
      // Parity of minute
      /*
      case 23:
        if (((getBits(timeCode.code >> 52) & 0xff) % 2) != ((timeCode.code >> 23) & 1)) {
          cout << "Minute Parity Error\n";
          sync = false;
        }
        break;
      */
      case 0:
        //sprintf(buf, "%02d:%02d, %03ddays, %2dyear, %1d Day of wees", 
        //  getHour(timeCode.code), getMinute(timeCode.code), getDay(timeCode.code), getYear(timeCode.code), getDayOfWeek(timeCode.code));
        cout << getHour(timeCode.code) << ":" << getMinute(timeCode.code) << "\n";
        cout << "Day:" << getDay(timeCode.code) << ",Year:" << getYear(timeCode.code) << ",DayOfWeek:" << getDayOfWeek(timeCode.code) << "\n";
        for (int i = 59; i >= 0; i--) {
          if (timeCode.code & (1LL << i)) {
            cout << "1";
          } else {
            cout << "0";
          }
        }
        cout << "\n";
        cout << timeCode.code << "\n";
        currentPosition = 60;
        break;
    }
  } else {
    if (previousCode == JJYCODE_M && currentCode == JJYCODE_M) {
      sync = true;
      currentPosition = 60;
      //timeCode.code = 60;
      cout << "Sync pos detect!\n";
    }
  }
  previousCode = currentCode;
}
