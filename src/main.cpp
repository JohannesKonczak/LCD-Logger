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

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/
                                         U8X8_PIN_NONE);   // All Boards without Reset of the Display

int LOG = 1;
int CS_PIN = 4;
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
char serial_in;
char buff[BUFF_MAX];

/**
 * Does something?
 */
void LOG_ON_OFF() {
    LOG = digitalRead(5);
    delay(100);

}

void parse_cmd(char *cmd, int cmdsize) {
    uint8_t i;
    uint8_t reg_val;
    char buff[BUFF_MAX];
    struct ts t;

    // TssmmhhWDDMMYYYY aka set time
    if (cmd[0] == 84 && cmdsize == 16) {
        // TssmmhhWDDMMYYYY aka set time
        // T00720603012020
        t.sec = inp2toi(cmd, 1);
        t.min = inp2toi(cmd, 3);
        t.hour = inp2toi(cmd, 5);
        t.wday = inp2toi(cmd, 7);
        t.mday = inp2toi(cmd, 8);
        t.mon = inp2toi(cmd, 10);
        t.year = inp2toi(cmd, 12) * 100 + inp2toi(cmd, 14);
        DS3231_set(t);
        //Serial.println("OK");
    } else if (cmd[0] == 49 && cmdsize == 1) {  // "1" get alarm 1
        DS3231_get_a1(&buff[0], 59);
        //Serial.println(buff);
    } else if (cmd[0] == 50 && cmdsize == 1) {  // "2" get alarm 1
        DS3231_get_a2(&buff[0], 59);
        // Serial.println(buff);
    } else if (cmd[0] == 51 && cmdsize == 1) {  // "3" get aging register
        //Serial.print("aging reg is ");
        //Serial.println(DS3231_get_aging(), DEC);
    } else if (cmd[0] == 65 && cmdsize == 9) {  // "A" set alarm 1
        DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
        //ASSMMHHDD
        for (i = 0; i < 4; i++) {
            cur_time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // ss, mm, hh, dd
        }
        byte flags[5] = {0, 0, 0, 0, 0};
        DS3231_set_a1(cur_time[0], cur_time[1], cur_time[2], cur_time[3], flags);
        DS3231_get_a1(&buff[0], 59);
        // Serial.println(buff);
    } else if (cmd[0] == 66 && cmdsize == 7) {  // "B" Set Alarm 2
        DS3231_set_creg(DS3231_INTCN | DS3231_A2IE);
        //BMMHHDD
        for (i = 0; i < 4; i++) {
            cur_time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // mm, hh, dd
        }
        byte flags[5] = {0, 0, 0, 0};
        DS3231_set_a2(cur_time[0], cur_time[1], cur_time[2], flags);
        DS3231_get_a2(&buff[0], 59);
        //Serial.println(buff);
    } else if (cmd[0] == 67 && cmdsize == 1) {  // "C" - get temperature register
        //Serial.print("temperature reg is ");
        //Serial.println(DS3231_get_treg(), DEC);
    } else if (cmd[0] == 68 && cmdsize == 1) {  // "D" - reset status register alarm flags
        reg_val = DS3231_get_sreg();
        reg_val &= B11111100;
        DS3231_set_sreg(reg_val);
    } else if (cmd[0] == 70 && cmdsize == 1) {  // "F" - custom fct
        reg_val = DS3231_get_addr(0x5);
        // Serial.print("orig ");
        //Serial.print(reg_val,DEC);
        //Serial.print("month is ");
        //Serial.println(bcdtodec(reg_val & 0x1F),DEC);
    } else if (cmd[0] == 71 && cmdsize == 1) {  // "G" - set aging status register
        DS3231_set_aging(0);
    }
}


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
}also

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

void initializeSD() {
    pinMode(CS_PIN, OUTPUT);

    if (SD.begin()) {
    } else {
        return;
    }
}

int openFileToWrite(char filename[]) {
    file = SD.open(filename, FILE_WRITE);

    if (file) {
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
        u8g2.setFont(u8g2_font_6x10_tf);
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

void setup() {

    Serial.begin(115200);
    u8g2.begin();
    Wire.begin();
    initializeSD();

    DS3231_init(DS3231_INTCN);
    memset(recv, 0, BUFF_MAX);
    // TssmmhhWDDMMYYYY aka set time
    //parse_cmd("T002315729072018",16); //Set time

    pinMode(CS_PIN, OUTPUT);
    pinMode(5, INPUT_PULLUP);

}

void loop() {

    if (LOG == 0) {

        LOG_ON_OFF();
        //serial_Input();
        //Erfassung Messwerte ();
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


        /*
        serial_Input()	{
                            if (Serial.available() > 0) {
                            serial_in = Serial.read();

                                                            if ((serial_in == 10 || serial_in == 13) && (recv_size > 0)) {
                                                                                                                parse_cmd(recv, recv_size);
                                                                                                                recv_size = 0;
                                                                                                                recv[0] = 0;
                                                            } else if (serial_in < 48 || serial_in > 122) {;                                              // ignore ~[0-9A-Za-z]
                                                            } else if (recv_size > BUFF_MAX - 2) {                                          // drop lines that are too long
                                                                                                                                            // drop
                                                                                                    recv_size = 0;
                                                                                                    recv[0] = 0;
                                                            } else if (recv_size < BUFF_MAX - 2) {
                                                                                                    recv[recv_size] = serial_in;
                                                                                                    recv[recv_size + 1] = 0;
                                                                                                    recv_size += 1;
                                                                                                }

                                                        }



                        }
            */

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