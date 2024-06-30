/*
 * pin 1 - not used          |  Micro SD card     |
 * pin 2 - CS (SS)           |                   /
 * pin 3 - DI (MOSI)         |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - SCK (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 - DO (MISO)         | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - not used          |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Connections for     ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * full-sized          ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * SD card             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Pin name         |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pin number    |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  █/
 *                  |__▍___▊___█___█___█___█___█___█___/
 *
 * Note:  The SPI pins can be manually configured by using `SPI.begin(sck, miso, mosi, cs).`
 *        Alternatively, you can change the CS pin and use the other default settings by using `SD.begin(cs)`.
 *
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SPI Pin Name | ESP8266 | ESP32 | ESP32‑S2 | ESP32‑S3 | ESP32‑C3 | ESP32‑C6 | ESP32‑H2 |
 * +==============+=========+=======+==========+==========+==========+==========+==========+
 * | CS (SS)      | GPIO15  | GPIO5 | GPIO34   | GPIO10   | GPIO7    | GPIO18   | GPIO0    |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DI (MOSI)    | GPIO13  | GPIO23| GPIO35   | GPIO11   | GPIO6    | GPIO19   | GPIO25   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DO (MISO)    | GPIO12  | GPIO19| GPIO37   | GPIO13   | GPIO5    | GPIO20   | GPIO11   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SCK (SCLK)   | GPIO14  | GPIO18| GPIO36   | GPIO12   | GPIO4    | GPIO21   | GPIO10   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 *
 * For more info see file README.md in this library or on URL:
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 */

/*
Uncomment and set up if you want to use custom pins for the SPI communication
#define REASSIGN_PINS
int sck = -1;
int miso = -1;
int mosi = -1;
int cs = -1;
*/

// set to 0 to test with SD lib (without SdFat)
#define USE_SDFAT 0

#include "Arduino.h"
#include <ctime>

#if USE_SDFAT
#include "SD_SDFAT.h"
#define SD SDF
#else
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define SD SD
#endif

uint32_t listDir(fs::FS &fs, const char *dirname, uint8_t levels, bool print = true) {
    uint32_t count = 0;
    Serial.printf("Listing directory: %s\n", dirname);
    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return count;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return count;
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            if ( print ) {
                Serial.print("  DIR : ");
                Serial.println(file.name());
            }
            count++;
            if (levels) {
                count += listDir(fs, file.path(), levels - 1, print);
            }
        }
        else {
            if ( print ) {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("  SIZE: ");
                Serial.println(file.size());
            }
            count++;
        }
        file = root.openNextFile();
    }
    return count;
}

void createDir(fs::FS &fs, const char *path) {
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) {
        Serial.println("Dir created");
    }
    else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char *path) {
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path)) {
        Serial.println("Dir removed");
    }
    else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char *path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    }
    else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("Message appended");
    }
    else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    }
    else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path) {
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) {
        Serial.println("File deleted");
    }
    else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char *path) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file) {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len) {
            size_t toRead = len;
            if (toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %lu ms\n", flen, end);
        file.close();
    }
    else {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
    file.close();
}

void testDir(fs::FS &fs, const char *path) {
    File f;
    if (f = fs.open(path)) {
        Serial.printf("\n **** testDir '%s', isDirectory()= %d\n", f.path(), f.isDirectory());
        f.rewindDirectory();

        Serial.println("\n+++++ getNextFileName ++++++");
        String filename;
        bool dir;
        while ((filename = f.getNextFileName(&dir)) != "") {
            Serial.printf("%s%s\n", dir ? "DIR:   " : "FILE:  ", filename.c_str());
        }

        Serial.println("\n+++++ openNextFile ++++++");
        f.rewindDirectory();
        File f1;
        // set timezone
        setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0", 3);  // Europa/Berlin
        tzset();
        while (1) {
            f1 = f.openNextFile();
            if (f1) {
                Serial.printf("%s, Size: %d  -", f1.path(), f1.size());

                time_t t = f1.getLastWrite();
                struct tm *tmstruct = localtime(&t);

                Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d   /   ", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday,
                              tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
                Serial.printf("ctime(): %s", ctime(&t));
                f1.close();
            }
            else {
                Serial.println("openNextFile(): no moore files or openNext fails");
                break;
            }
        }
        f.close();
    }
    else
        Serial.println("testDir() - open failed");
}

/***********************************************************************************************/

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

#if USE_SDFAT
    // use dedicated SPI for much better performance
    SPIClass SD_SPI(VSPI);
    #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(25), &SD_SPI)
    if (!SD.begin(SD_CONFIG)) {
#else
#ifdef REASSIGN_PINS
    SPI.begin(sck, miso, mosi, cs);
    if (!SD.begin(cs)) {
#else
    if (!SD.begin()) {
#endif
#endif  // USE_SDFAT
        
        Serial.println("Card Mount Failed");
        // return;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    }
    else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    // Serial.printf("sd Card Size: %llu MB\n", cardSize);   // %llu can not used if SDK Configuration had 'CONFIG_NEWLIB_NANO_FORMAT=y'
    Serial.printf("SD Card Size: %luMB\n", (uint32_t)cardSize);

    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");

    uint32_t start = millis();
    uint32_t end = start;
    uint32_t cnt = listDir(SD, "/", 3, true);
    end = millis() - start;
    Serial.printf("listDir takes %lu ms for %lu files/dirs\n", end, cnt);

    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    //testDir(SD, "/"); // shows timestamp lastWrite 

    Serial.printf("Total space: %luMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %luMB\n", SD.usedBytes() / (1024 * 1024));
}

/*  
benchmarks examples (SD_SDFAT vs SD):
    USE_SDFAT 0: 
        listDir takes 27702 ms for 674 files/dirs (27975 ms with printing filenames)
        Read from file: Hello World!
        1048576 bytes read for 2452 ms
        1048576 bytes written for 6242 ms

    USE_SDFAT 1:
        listDir takes 710 ms for 537 files/dirs (2937 ms with printing filenames)
        Read from file: Hello World!
        1048576 bytes read for 496 ms
        1048576 bytes written for 590 ms
*/

void loop() {}
