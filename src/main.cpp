#include <Arduino.h>
#include <Wire.h>

#include "logfile.h"
#include "display.h"

#define PROCESSING_INTERVAL 1 // Log-Intervall in Sekunden
#define INTERVAL PROCESSING_INTERVAL * 1000

// Hier die Analog Pins eintragen, an denen die Messwerte erhoben werden.
#define INPUT_1 A0
#define INPUT_2 A0
#define INPUT_3 A0
#define INPUT_4 A0

#define CS_PIN (uint8_t)4
// rechnet sensorwert in mA um
#define INT_TO_MIL_AMPS(x) ((x) * 0.019550342)

// status, ob auf sd karte geschrieben werden soll
bool logToSD = false;
// zeit (in ms seit programmstart) zu der der letzte logeintrag erfolgt ist
unsigned long lastWrite = 0;

/**
 * Wird aufgerufen, wenn bei Pin 5 das Signal von HIGH auf LOW wechselt.
 */
void toggleSDLogging() {
    logToSD = !logToSD;
}

/**
 * Rechnet double-messwert in "string" um.
 * @param value messwert
 * @return string-repräsentation des messwertes
 */
char* doubleToLogMessage(double value) {
    char *result = (char*) malloc(5*sizeof(char));
    dtostrf(value, 4, 1, result);
    return result;
}

void setup() {
    Serial.begin(115200);
    initializeDisplay();
    if(!initializeLogfile(CS_PIN)) {
        // TODO error
    }
    pinMode(5, INPUT_PULLUP);
    // registriert den interrupt handler "toggleSDLogging" für pin 5
    attachInterrupt(digitalPinToInterrupt(5), toggleSDLogging, FALLING);
}

void loop() {
    double sensorData[4];
    sensorData[0] = INT_TO_MIL_AMPS(analogRead(INPUT_1));
    sensorData[1] = INT_TO_MIL_AMPS(analogRead(INPUT_2));
    sensorData[2] = INT_TO_MIL_AMPS(analogRead(INPUT_3));
    sensorData[3] = INT_TO_MIL_AMPS(analogRead(INPUT_4));
    // und so weiter, die pins sind allerdings komisch benannt?

    // legt 4 addressen zu strings an
    char* dataString[4];

    // generiert den string und speichert dessen addresse
    // für jeden messwert
    for(int i = 0; i < 4; i++) {
        dataString[i] = doubleToLogMessage(sensorData[i]);
        Serial.println(dataString[i]);
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
    Serial.println("Displaying...");
    displaySensorData(dataString, logToSD);
    delay(3000);
}