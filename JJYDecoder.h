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
    struct timeCode_t{};
    long timeHigh;
    long timeLow;
    int getMinute(long long code);
    int getHour(long long code);
    int getDay(long long code);
    int getYear(long long code);
    int getDayOfWeek(long long code);
    int getBits(unsigned char value);
    enum JJYCODE{};
    static JJYCODE previousCode;
    static int currentPosition;
    static int sync;
    static struct timeCode_t timeCode;

public:
    void intChange();
};

#endif