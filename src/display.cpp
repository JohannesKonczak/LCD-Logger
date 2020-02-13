#include "display.h"

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

void initializeDisplay() {
    u8g2.begin();
}

/**
 * Zeigt Sensordaten auf Display an.
 * @param sensorData array mit strings, die sensorwerte repräsentieren. Muss 4 werte enthalten!
 * @param isLogging einstellung, ob auf SD karte geschrieben wird oder nicht.
 */
void displaySensorData(char **sensorData, bool isLogging) {
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_6x10_tf);

        int y = 10;
        for(int i = 0; i < 4; i++) {
            char label[7];
            sprintf(label, "MW%d = ", i);

            u8g2.drawStr(10, y, label);
            u8g2.drawStr(45, y, sensorData[i]);
            u8g2.drawStr(75, y, "mA");
            y += 12;
        }

        u8g2.drawStr(80,22,isLogging ? "Log ON" : "Log OFF");

        /* display uhrzeit + Datum */
//        u8g2.drawStr(10, 60, "Zeit");
//        u8g2.drawStr(50, 60, "Datum");
    } while ( u8g2.nextPage() );
}