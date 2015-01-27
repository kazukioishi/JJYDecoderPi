#include <stdio.h>
#include <wiringPi.h>

#ifndef JJYDECODER_L
#define JJYDECODER_L

class JJYDecoder{
private:
    long markerMin = 50, markerMax = 350;
    long highMin = 350, highMax = 650;
    long lowMin = 650, lowMax = 950;
    static int pinF;
    static int pinTP;
    static int pinP;
    struct timeCode_t{};
    long timeHigh = 0;
    long timeLow = 0;
    int getMinute(long long code);
    int getHour(long long code);
    int getDay(long long code);
    int getYear(long long code);
    int getDayOfWeek(long long code);
    int getBits(unsigned char value);
    enum JJYCODE;
    static JJYCODE previousCode;
    static int currentPosition;
    static int sync;
    static struct timeCode_t timeCode;

public:
    void intChange();
};

#endif