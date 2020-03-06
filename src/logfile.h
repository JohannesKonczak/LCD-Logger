#ifndef LCD_LOGGER_LOGFILE_H
#define LCD_LOGGER_LOGFILE_H

#include <HardwareSerial.h>

#define LOGFILE "/log.csv"

bool initializeLogfile(int pin, HardwareSerial &serial);
void writeLogEntry(char* msg);

#endif //LCD_LOGGER_LOGFILE_H
