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
        unsigned long long code : 60;
    } _timeCode_t;
    long timeHigh;
    long timeLow;
    int getMinute(long long code);
    int getHour(long long code);
    int getDay(long long code);
    int getYear(long long code);
    int getDayOfWeek(long long code);
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
    static bool error;
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
