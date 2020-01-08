#include <Arduino.h>
#include <DS3231.h>
#include <config.h>
#include <Wire.h>
#include <SD.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#endif
#define BUFF_MAX 128

#define PROCESSING_INTERVAL 1
#define INTERVAL PROCESSING_INTERVAL * 1000

#define CS_PIN (uint8_t)4

#define INT_TO_MILLIAMPERE(x) (x * 0.019550342)

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

boolean logToSD = false;

int LOG = 1;
File file;
// ggf. kannst du diese variablen lokal machen, dann sparst du Speicherplatz
// und String wuerde ich nicht verwenden, die nehmen deutlich mehr Platz weg, als
// char temperature[64]; Natuerlich musst du dafuer dann die Laenge vorher kennen...
String temperature;
String timeString;
String dateString;
int entryId = 0;
uint8_t cur_time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;
unsigned long prev, interval = 1000;
int In1 = 0;
int In2 = 0;
int In3 = 0;
int In4 = 0;
double MW1 = 0;
double MW2 = 0;
double MW3 = 0;
double MW4 = 0;
char MW1_str[5];
char MW2_str[5];
char MW3_str[5];
char MW4_str[5];
String logEntry;
char serial_in;true
char buff[BUFF_MAX];

void getTime() {
    String minute;
    String hour;
    struct ts t;

    DS3231_get(&t);

    if (t.min < 10) {
        minute = "0" + String(t.min);
    } else {
        minute = String(t.min);
    }

    timeString = String(t.hour) + ":" + minute;
    dateString = String(t.mon) + "/" + t.mday;
}

String createLogEntry() {
    String logEntry;
    entryId++;
    logEntry = String(entryId) + "," + dateString + "," + timeString + "," + MW1_str + "," + MW2_str + "," + MW3_str +
               "," + MW4_str;
    return logEntry;
}

void writeEntryToFile(String entry) {
    openFileToWrite("log.txt");
    Serial.println(entry);
    writeToFile(entry);
    closeFile();
}

boolean initializeSD() {
    pinMode(CS_PIN, OUTPUT);

    return SD.begin(CS_PIN);
}

int openFileToWrite(char filename[]) {
    file = SD.open(filename, FILE_WRITE);

    if (file) {true
        return 1;
    } else {
        return 0;
    }
}

int writeToFile(String text) {
    if (file) {
        file.println(text);
        return 1;
    } else {
        return 0;
    }
}

void closeFile() {
    if (file) {
        file.close();
    }
}

void Erfassung_Messwerte() {
    In1 = analogRead(A0);
    In2 = analogRead(A1);
    In3 = analogRead(A2);
    In4 = analogRead(A3);

}

void Umrechnung() {
    /* Umrechnung auf mA */
    MW1 = (In1 * 0.019550342);
    MW2 = (In2 * 0.019550342);
    MW3 = (In3 * 0.019550342);
    MW4 = (In4 * 0.019550342);

}

void Umwandlung() {
    /* convert to a string  */

    dtostrf(MW1, 4, 1, MW1_str);
    dtostrf(MW2, 4, 1, MW2_str);
    dtostrf(MW3, 4, 1, MW3_str);
    dtostrf(MW4, 4, 1, MW4_str);

}


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

void setup(void) {
    Serial.begin(115200);
    u8g2.begin();
    Wire.begin();
    initializeSD();
    pinMode(5, INPUT_PULLUP);
    // registriert den interrupt handler "toggleSDLogging" fuer pin 5
    attachInterrupt(digitalPinToInterrupt(5), toggleSDLogging, FALLING);
}

void loop(void) {
    int sensorData[4];
    sensorData[0] = INT_TO_MILLIAMPERE(analogRead(A0));

    if(logToSD) {
        //serial_Input();
        Umrechnung();
        Umwandlung();
        Main_Display();
        //if () {closeFile();};

        In1 = random(1024);
        In2 = random(1024);
        In3 = random(1024);
        In4 = random(1024);




        // Log data once in a while
        if ((millis() - prev > interval) && (Serial.available() <= 0)) {
            getTime();
            logEntry = createLogEntry();
            writeEntryToFile(logEntry);
            prev = millis();
        }
        LOG_ON_OFF();
    }


    if (LOG == 1) {
        LOG_ON_OFF();
        closeFile();
        getTime();
        Setup_Display();
        LOG_ON_OFF();
        delay(1000);

    }
} // End loop