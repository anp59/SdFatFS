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
In this example, the compatibility of the two file system wrappers SDFAT_FS (for SdFat) 
and FS (for Arduino's own file systems) is tested. 
By setting one of the three TEST_CASE 1, 2 or 3, the performance values of the three
file systems SdFat, SD and SD_MMC can also be compared:

The examples was tested with a ESP32S3-devKit-C1-Board respectively an YB-ESP32S3 AMP-board and arduinoespressif32 V3.2.1

!! When switching between SPI-mode (SDFATFS or SD) and MMC-mode (TEST_SD_MMC), operating voltage from SD module has to be disconnected !!
*/

// ##########  Test selection  ##################
// set TEST_CASE to     1   -> test SDFATFS
//                      2   --> test SD_MMC
//                      3   --> test SD
#define TEST_CASE   1           

// An additional PlayList class was implemented for SDFatFS. The content from a specific directory is read in and stored internally. 
// The listDir method can be used to output this data in a similar way to listDir. This test can be activated with TEST_PLAYLIST = 1.
#define TEST_PLAYLIST   1 
#define LIST_PLAYLIST   0     // prints all files (complete file path) of the playlist
// ###############################################


#if (TEST_CASE > 3) || (TEST_CASE < 1) 
    #undef TEST_CASE
    #define TEST_CASE 1
    #warning TEST_CASE was set to 1 per default! Please select the test you want. 
#endif

#if TEST_CASE == 1
    #include "SD_SDFAT.h"
    #include "SdFatPlayList.h"
    #define SD SDF
    #define TEST_NAME "Test SDFatFS"
    #if TEST_PLAYLIST        
        SdFatPlayList plist;
    #endif
#elif TEST_CASE == 2
    #include "SD_MMC.h"   
    #include "FS.h"
    #define SD SD_MMC
    #define TEST_NAME "Test SD_MMC"
#elif TEST_CASE == 3
    #include "SD.h"
    #include "FS.h"
    #define TEST_NAME "Test SD"
#endif

#if CONFIG_IDF_TARGET_ESP32S3
    #if TEST_CASE == 2    
        #define SD_MMC_D0   13       // miso
        #define SD_MMC_CLK  12      // clk 
        #define SD_MMC_CMD  11      // mosi
    #else
        SPIClass SD_SPI(FSPI);
    #endif
#endif

#if CONFIG_IDF_TARGET_ESP32
    #if TEST_CASE == 2    
        #define SD_MMC_D0   2       // miso
        #define SD_MMC_CLK  14      // clk 
        #define SD_MMC_CMD  15      // mosi
    #else
        SPIClass SD_SPI(VSPI);
    #endif
#endif


// global variables

const char *rootDir = "/";	// rootDir for listDir... 
uint8_t levels = 10;		// scan depth (0: only rootDir)

typedef struct {
    uint16_t dcount;    // dirs counter   
    uint16_t fcount;    // files counter
} list_count_t;


// function prototypes

void listDir(fs::FS &fs, const char *dirname, uint8_t levels, list_count_t& lc);
void createDir(fs::FS &fs, const char *path);
void removeDir(fs::FS &fs, const char *path);
void readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void renameFile(fs::FS &fs, const char *path1, const char *path2);
void deleteFile(fs::FS &fs, const char *path);
void testFileIO(fs::FS &fs, const char *path);
void testDir(fs::FS &fs, const char *path);
void createDirWithOpen(fs::FS &fs, const char *path);
void printCardType(sdcard_type_t cardType);


/*********************************  SETUP  **************************************************************/

void setup() {
    Serial.begin(115200);
    while (!Serial)
        delay(10);
    Serial.println();

    Serial.printf("\n******* %s *******\n", TEST_NAME);
#if TEST_CASE == 1
    // SPI CLK 50 MHZ was successfully tested with the YB-ESP32-S3 AMP board from yellobyte. If problems occur, the CLK frequency should be reduced to 25 or 16 MHz.
    #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(40), &SD_SPI)
    if (!SD.begin(SD_CONFIG)) {
        Serial.println("Card Mount Failed");
        return;
    }
#elif TEST_CASE == 2
    SD.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    // const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false, int sdmmc_frequency = BOARD_MAX_SDMMC_FREQ /*!kHz*/, uint8_t maxOpenFiles = 5
    // SDMMC_FREQ_52M -> max 52 MHz
    if(!SD.begin( "/sdmmc", true)) {
        Serial.println("Card Mount Failed");
        return;
    }
#elif TEST_CASE == 3  	
        // bool begin(uint8_t ssPin = (uint8_t)10U, SPIClass &spi = SPI, uint32_t frequency = 4000000U, const char *mountpoint = "/sd", uint8_t max_files = (uint8_t)5U, bool format_if_empty = false)
        //if (!SD.begin()) {
        if (!SD.begin(SS, SPI, 40 * 1000000, "/sd", 20)) { // setup with max SPI frequency  
            Serial.println("Card Mount Failed");
            return;
        }
#else
        Serial.println("No Test selected!");
        return;
#endif
   
// ************* START Test *****************
    
printCardType(SD.cardType());
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    // Serial.printf("\nSD Card Size: %llu MB\n", cardSize);   // %llu can not used if SDK Configuration had 'CONFIG_NEWLIB_NANO_FORMAT=y'
    Serial.printf("SD Card Size: %luMB\n", (uint32_t)cardSize);
    
    Serial.println("Test listDir...");    
    list_count_t lcnt;    
    uint32_t start = millis();
    uint32_t end = start;
    lcnt.dcount = lcnt.fcount = 0;
    listDir(SD, rootDir, levels, lcnt);
    end = millis() - start;
    Serial.printf("  processing listDir for %s takes %lu ms for %u files in %u /dirs\n", TEST_NAME, end, lcnt.fcount, lcnt.dcount);
    
#if (TEST_CASE == 1 && TEST_PLAYLIST)
    Serial.println("Test building playlist, based on SdFatFS ...");    
    start = millis();
    end = start;
    // With SDFat it is possible to evaluate the “system” and “hidden” attributes of files and directories. 
    // These are ignored by default when the playlist is created. 
    // This can be switched off to obtain the same results as in listDir. 
    plist.ignoreSystemAndHiddenFiles(false);
    // Directories without files (including their subdirectories) are not saved in the playlist by default.
    // This can be switched off to obtain the same results as in listDir. 
    plist.ignoreEmptyDirPaths(false);
    plist.create(rootDir, levels);
    end = millis() - start;
    
    #if LIST_PLAYLIST
        Serial.printf("\n*************  List %d Files of the playlist ************\n", plist.fileCount());
        for (size_t i = 0; i < plist.fileCount(); i++)
            Serial.printf("%05u: %s\n", i, plist.getFilePathAtIdx(i));
        Serial.println();
    #endif
    
    Serial.printf("  creating playlist takes %lu ms for %u files in %u /dirs\n", end, plist.fileCount(), plist.dirCount()); 
#endif


Serial.println();
    createDir(SD, "/mydir");
    createDirWithOpen(SD, "/mydir/open.dir/");  // if '/' at the end of path the "open.dir" will be created as directory, works for SDFAT_FS.  
                                                // For USE_MMC and SD this feature requires the FS-Library version 3.0.0 or higher. 
                                                // With V2.x no directories with open can be generated (error due to attached slash)                                  
    removeDir(SD, "/mydir");
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    //optional:
    //testDir(SD, "/");
    
    //Delete the comment chars if needed:
    //testDir(SD, "/");       // shows timestamp lastWrite for all files in root.
    // Serial.printf("Total space: %luMB\n", SD.totalBytes() / (1024 * 1024));
    // Serial.printf("Used space: %luMB\n", SD.usedBytes() / (1024 * 1024));
    Serial.println("**** READY ****");
}

/*********************************  LOOP  **************************************************************/
void loop() {}



//#######################################################################################################

void listDir(fs::FS &fs, const char *dirname, uint8_t levels, list_count_t& lc) {
    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }
    //Serial.printf("Scan directory(%03d): %s\n", lc.dcount, dirname);
    lc.dcount++;
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            // Serial.printf("  DIR : %s\n",file.name());
            if (levels) {
                listDir(fs, file.path(), levels - 1, lc);
            }
        }
        else {
            lc.fcount++;
            //Serial.printf("  FILE (%04d): %s\n", lc.fcount, file.name());
        }
        file = root.openNextFile();
    }
}


void createDir(fs::FS &fs, const char *path) {
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) {
        Serial.println("  Dir created");
    }
    else {
        Serial.println("  mkdir failed");
    }
}


void removeDir(fs::FS &fs, const char *path) {
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path)) {
        Serial.println("  Dir removed");
    }
    else {
        Serial.println("  rmdir failed");
    }
}


void readFile(fs::FS &fs, const char *path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file) {
        Serial.println("  Failed to open file for reading");
        return;
    }

    Serial.print("  Read from file: ");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}


void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("  Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("  File written");
    }
    else {
        Serial.println("  Write failed");
    }
    file.close();
}


void appendFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("  Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("  Message appended");
    }
    else {
        Serial.println("  Append failed");
    }
    file.close();
}


void renameFile(fs::FS &fs, const char *path1, const char *path2) {
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("  File renamed");
    }
    else {
        Serial.println("  Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path) {
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) {
        Serial.println("  File deleted");
    }
    else {
        Serial.println("  Delete failed");
    }
}


void testFileIO(fs::FS &fs, const char *path) {
    Serial.printf("TestFileIO file: %s\n", path);
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
        Serial.printf("  %u bytes read for %lu ms\n", flen, end);
        file.close();
    }
    else {
        Serial.println("  Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("  Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("  %u bytes written for %lu ms\n", 2048 * 512, end);
    file.close();
}


void testDir(fs::FS &fs, const char *path) {
    File f;
    if (f = fs.open(path)) {
        Serial.printf("\nTestDir '%s', isDirectory()= %d\n", f.path(), f.isDirectory());
        f.rewindDirectory();
        Serial.println("  ---> test getNextFileName()...");
        String filename;
        bool dir;
        while ((filename = f.getNextFileName(&dir)) != "") {
            Serial.printf("    %s%s\n", dir ? "DIR:   " : "FILE:  ", filename.c_str());
        }

        Serial.println("  ---> test openNextFile()...");
        f.rewindDirectory();
        File f1;
        // set timezone
        setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0", 3);  // Europa/Berlin
        tzset();
        while (1) {
            f1 = f.openNextFile();
            if (f1) {
                Serial.printf("%s\n\tSize: %08u byte", f1.path(), f1.size());

                time_t t = f1.getLastWrite();
                struct tm *tmstruct = localtime(&t);

                Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday,
                              tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
                Serial.printf("\tctime(): %s", ctime(&t));
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


void createDirWithOpen(fs::FS &fs, const char *path) {
    Serial.printf("CreateDirWithOpen: path: %s\n", path);
    File f = fs.open(path, FILE_WRITE, true);  
    if (fs.exists(path)) {
        Serial.printf("  path '%s' is created as %s\n", path, f.isDirectory() ? "dir" : "file"); 
        if (f.isDirectory() ) {
            if (fs.rmdir(path))
                Serial.printf("  dir '%s' was removed\n", path);
        }
        else {
            if (fs.remove(path))
                Serial.printf("  file '%s' was removed\n", path);
        }
    }
    else
        Serial.printf("  open / creation failed\n");
    f.close();
}


void printCardType(sdcard_type_t cardType) {
        if (cardType == CARD_NONE) {
            Serial.println("No SD card attached");
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
    }
