#ifndef LCD_LOGGER_LOGFILE_H
#define LCD_LOGGER_LOGFILE_H

#define LOGFILE "log.csv"

bool initializeLogfile(int pin);
void writeLogEntry(char* msg);

#endif //LCD_LOGGER_LOGFILE_H
