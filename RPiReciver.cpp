#include <cstdio>
#include <iostream>
#include <ctime>
using namespace std;
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "JJYDecoder.h"

JJYDecoder *decoder;

void sig_handler(int signo){
    cout << "ABORTING..." << "\n";
    delete decoder;
}

void TimeReciveCallback(struct std::tm stm){
    char timechar[20];
    strftime(timechar,20,"%Y-%m-%d %H:%M:%S",&stm);
    cout << timechar << "\n";
}

int main(int argc, char* argv[]){
    try{
        signal(SIGTERM, sig_handler); //on stop
        decoder = new JJYDecoder();
        decoder->OnTimeRecive = &TimeReciveCallback;
        while(true){
            //sleep(1);
        }
        return 0;
    }catch (char *str) {
        cout << "ERROR" << "\n";
        cout << str << "\n";
        return 1;
    }
}
