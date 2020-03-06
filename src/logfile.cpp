#include "logfile.h"
#include <SPI.h>
#include <SD.h>

bool initializeLogfile(int pin) {
    pinMode(pin, OUTPUT);
    bool status = false;
    do {
        delay(500);
        status = SD.begin(pin);
    } while(!status);
    return status;
}

File openFile() {
    File f = SD.open(LOGFILE, FILE_WRITE);
    return f;
}

void writeLogEntry(char* msg) {
    File f = openFile();
    f.println(msg);
    f.close();
}