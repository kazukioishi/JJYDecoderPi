#include <cstdio>
#include <iostream>
using namespace std;
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "JJYDecoder.h"

JJYDecoder *decoder;

void sig_handler(int signo){
    delete decoder;
}

int main(int argc, char* argv[]){
    try{
        signal(SIGTERM, sig_handler); //on stop
        decoder = new JJYDecoder();
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