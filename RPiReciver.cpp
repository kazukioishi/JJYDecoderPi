#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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
        while(0){}
        return 0;
    }catch (char *str) {
        println(str);
        return 1;
    }
}