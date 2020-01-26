//
// Created by alex on 26/01/2020.
//

#include "logfile.h"
#include <SPI.h>
#include <SD.h>

bool initializeLogfile(int pin) {
    pinMode(pin, OUTPUT);
    return SD.begin(pin);
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