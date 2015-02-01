#include <cstdio>
#include <iostream>
using namespace std;
#include <wiringPi.h>

#ifndef JJYDECODER_L
#define JJYDECODER_L

class JJYDecoder{
private:
    long markerMin, markerMax;
    long highMin, highMax;
    long lowMin, lowMax;
    static int pinF;
    static int pinTP;
    static int pinP;
    struct timeCode_t {
        unsigned long code : 60;
    } _timeCode_t;
    long timeHigh;
    long timeLow;
    int getMinute(long code);
    int getHour(long code);
    int getDay(long code);
    int getYear(long code);
    int getDayOfWeek(long code);
    int getBits(unsigned char value);
    enum JJYCODE{
        JJYCODE_M, //Marker
        JJYCODE_H, //High
        JJYCODE_L //Low
    };
    char* getS(JJYCODE c);
    static JJYCODE previousCode;
    static int currentPosition;
    static bool sync;
    static struct timeCode_t timeCode;
    static int LastHighLow;
public:
    void intChange();
    JJYDecoder();
    ~JJYDecoder();
    static void StaticEventCaller();
};

extern JJYDecoder *DecoderSingleton;
#endif
