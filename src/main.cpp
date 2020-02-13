#include <Arduino.h>
#include <RtcDS3231.h>
#include <config.h>
#include <Wire.h>
#include <SD.h>
#include <U8g2lib.h>

#include "logfile.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#define BUFF_MAX 128

#define PROCESSING_INTERVAL 1 // Log-Intervall in Sekunden
#define INTERVAL PROCESSING_INTERVAL * 1000

#define CS_PIN (uint8_t)4

#define INT_TO_MIL_AMPS(x) ((x) * 0.019550342)

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

// status, ob auf sd karte geschrieben werden soll
bool logToSD = false;
// zeit (in ms seit programmstart) zu der der letzte logeintrag erfolgt ist
unsigned long lastWrite = 0;

void Main_Display() {
    u8g2.firstPage();
    do {
        /* display MW1 */
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 10, "MW1 = ");
        u8g2.drawStr(45, 10, MW1_str);
        u8g2.drawStr(75, 10, "mA");

        /* display MW2 */
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 22, "MW2 = ");
        u8g2.drawStr(45, 22, MW2_str);
        u8g2.drawStr(75, 22, "mA");

        /* display MW3 */
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 34, "MW3 = ");
        u8g2.drawStr(45, 34, MW3_str);
        u8g2.drawStr(75, 34, "mA");

        /* display MW4 */
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 46, "MW4 = ");
        u8g2.drawStr(45, 46, MW4_str);
        u8g2.drawStr(75, 46, "mA");

        /* display uhrzeit + Datum */
        u8g2.setFont(u8g2_font_6x10_tdigitalRead(5)f);
        u8g2.drawStr(10, 60, "Zeit");
        u8g2.drawStr(50, 60, "Datum");

        /* display Log ON / OFF */
        //         u8g2.setFont(u8g2_font_6x10_tf);
        //         if (1 == 1){u8g2.drawStr(85,22,"Log ON");}
        //        else {u8g2.drawStr(80,22,"Log OFF");};

        /* display Log OFF */
        //        u8g2.setFont(u8g2_font_6x10_tf);
        //        u8g2.drawStr(85,10,"SD OK");





    } while (u8g2.nextPage());
    // delay(1000);

}

void Setup_Display() {
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 10, "Uhrzeit");

        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 22, "Datum");

        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 34, "LOG Passiv");

        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 46, "MW4 = ");
        u8g2.drawStr(75, 46, "mA");

        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 58, "Schalter umschalten");
        u8g2.drawStr(10, 70, "um LOG zu Starten");

        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 82, "LOG Intervall");
        u8g2.drawStr(45, 82, "Wert");
        u8g2.drawStr(50, 82, "Einheit");


    } while (u8g2.nextPage());
    // delay(1000);

}

/**
 * Wird aufgerufen, wenn bei Pin 5 das Signal von HIGH auf LOW wechselt.
 */
void toggleSDLogging() {
    logToSD = !logToSD;
}

char* doubleToLogMessage(double value) {
    char *result = (char*) malloc(5*sizeof(char));
    dtostrf(value, 4, 1, result);
    return result;
}

void setup() {
    Serial.begin(115200);
    u8g2.begin();
    Wire.begin();
    if(!initializeLogfile(CS_PIN)) {
        // TODO error
    }
    pinMode(5, INPUT_PULLUP);
    // registriert den interrupt handler "toggleSDLogging" für pin 5
    attachInterrupt(digitalPinToInterrupt(5), toggleSDLogging, FALLING);
}

void loop() {
    double sensorData[4];
    sensorData[0] = INT_TO_MIL_AMPS(analogRead(A0));
    sensorData[1] = INT_TO_MIL_AMPS(analogRead(A3));
    // und so weiter, die pins sind allerdings komisch benannt?

    // legt 4 addressen zu strings an
    char* dataString[4];

    // generiert den string und speichert dessen addresse
    // für jeden messwert
    for(int i = 0; i < 4; i++) {
        dataString[i] = doubleToLogMessage(sensorData[i]);
    }

    // schreiben auf sd karte
    if(logToSD) {
        if((millis() - lastWrite > INTERVAL)) {
            lastWrite = millis();
            char *fullMessage = (char*) malloc(5*4*sizeof(char));
            sprintf(fullMessage, "%s;%s;%s;%s", dataString[0], dataString[1], dataString[2], dataString[3]);
            writeLogEntry(fullMessage);
        }
    }

    // anzeige der sensordaten auf display
    displaySensorData();
}