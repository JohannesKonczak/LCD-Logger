//
// Created by alex on 13/02/2020.
//

#ifndef LCD_LOGGER_DISPLAY_H
#define LCD_LOGGER_DISPLAY_H

void initializeDisplay();
void displaySensorData(char **sensorData, bool isLogging, bool sdStatus);

#endif //LCD_LOGGER_DISPLAY_H
