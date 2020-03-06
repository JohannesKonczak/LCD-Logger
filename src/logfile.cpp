#include "logfile.h"
#include <SPI.h>
#include <SD.h>

bool initializeLogfile(int pin, HardwareSerial &serial) {
    pinMode(pin, OUTPUT);
    bool status = false;
    do {
        delay(500);
        status = SD.begin(pin);
    } while(!status);
    serial.printf("SD Card: %d | %llu\n", SD.cardType(), SD.cardSize());
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