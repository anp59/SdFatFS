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
By setting the two defines USE_SDFATFS and USE_SDMMC, the performance values of the three
file systems SdFat, SD and SD_MMC can also be compared:

            USE_SDFATFSUSE  USE_SD_MMC
                SD  | SD_MMC |  SdFat
--------------------+---------+---------                
USE_SDFATFS:    0   |  0     |   1      
USE_SD_MMC:     0   |  1     |   0

The example was tested with an ESP32S3-DevBoard and ArduinoESP32 V2.17
 
 */

// !! when switching between SPI-mode (USE_SDFATFS) and MMC-mode (USE_SD_MMC), operating voltage from SD module has to be disconnected !!
#define    USE_SDFATFS 1
#if !USE_SDFATFS
    #define USE_SD_MMC 0
#else
    // USE_SD: set USE_SDFATFS == 0,  USE_SD_MMC == 0 
#endif

//##########################################################################################################

#if USE_SDFATFS
    #include "SD_SDFAT.h"
    #include "SdFat.h"
    #include "SdFatPlayList.h"

    #define SD SDF        
    SdFatPlayList plist;
#else
    #if USE_SD_MMC
        #include "SD_MMC.h"
        #define SD SD_MMC
    #else
        #include "SD.h"
    #endif
    #include "FS.h"
#endif

#if CONFIG_IDF_TARGET_ESP32S3
    #if USE_SD_MMC    
        #define SD_MMC_D0   13       // miso
        #define SD_MMC_CLK  12      // clk 
        #define SD_MMC_CMD  11      // mosi
    #else
        SPIClass SD_SPI(FSPI);
    #endif
#endif

#if CONFIG_IDF_TARGET_ESP32
    #if USE_SD_MMC    
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
bool printData = false;		// true: prints all files in listDir...

typedef struct {
    uint16_t dcount;    
    uint16_t fcount;
} list_count_t;

// function prototypes
#if USE_SDFATFS
const char* _name(File32& f, const char *path = nullptr);
void listDir_SdFat(fs::FS &fs, const char * dirname, uint8_t levels, list_count_t& lc, bool list_hidden_files = true);
#endif
void listDir(fs::FS &fs, const char *dirname, uint8_t levels, list_count_t& lc, bool ignore_hidden_files = true);
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

/***********************************************************************************************/

void setup() {
    Serial.begin(115200);
    while (!Serial)
        delay(10);
    Serial.println();

#if USE_SDFATFS
    #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(25), &SD_SPI)
    Serial.println("\n** USING SDFATFS **");
    if (!SD.begin(SD_CONFIG)) {
        Serial.println("Card Mount Failed");
        return;
    }
#else    
    #if USE_SD_MMC
        SD_MMC.setPins(SCK, MOSI, MISO);
        // const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false, int sdmmc_frequency = BOARD_MAX_SDMMC_FREQ /*!kHz*/, uint8_t maxOpenFiles = 5
        // SDMMC_FREQ_52M -> max 52 MHz
        Serial.println("\n** USING SD_MMC **");
        if(!SD_MMC.begin( "/sdmmc", true)) {
            Serial.println("Card Mount Failed");
            return;
        }
    #else   	
        // bool begin(uint8_t ssPin = (uint8_t)10U, SPIClass &spi = SPI, uint32_t frequency = 4000000U, const char *mountpoint = "/sd", uint8_t max_files = (uint8_t)5U, bool format_if_empty = false)
        Serial.println("\n** USING SD **");
        //if (!SD.begin()) {
        if (!SD.begin(SS, SPI, 25000000U, "/sd", 20)) { // setup with max SPI frequency  
            Serial.println("Card Mount Failed");
            return;
        }
    #endif
#endif
   
    sdcard_type_t cardType = SD.cardType();
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

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    // Serial.printf("sd Card Size: %llu MB\n", cardSize);   // %llu can not used if SDK Configuration had 'CONFIG_NEWLIB_NANO_FORMAT=y'
    Serial.printf("SD Card Size: %luMB\n", (uint32_t)cardSize);
    
    Serial.println("listDir...");    
    list_count_t lcnt, lcnt1, lcnt2;    
    uint32_t start = millis();
    uint32_t end1 = start;
    lcnt.dcount = lcnt.fcount = 0;
    listDir(SD, rootDir, levels, lcnt);
    end1 = millis() - start;
    #if USE_SD_MMC
      Serial.printf("listDir with SD_MMC takes %lu ms for %u files in %u /dirs\n", end1, lcnt.fcount, lcnt.dcount);
    #else
        #if USE_SDFATFS
            Serial.printf("  listDir with SDFAT_FS takes %lu ms for %u files in %u /dirs\n", end1, lcnt.fcount, lcnt.dcount);
        #else
            Serial.printf("listDir with SD takes %lu ms for %u files in %u /dirs\n", end1, lcnt.fcount, lcnt.dcount);
        #endif
    #endif
#if USE_SDFATFS
    Serial.println("\nOriginal SdFat will be used...");
    Serial.println("  listDir_SdFat...");
    start = millis();
    uint32_t end2 = start;
    lcnt1.dcount = lcnt1.fcount = 0;
    //plist.files.clear();
    listDir_SdFat(SD, rootDir, levels, lcnt1);
    end2 = millis() - start;
    Serial.printf("    listDir with SdFat takes %lu ms for %u files in %u /dirs\n", end2, lcnt1.fcount, lcnt1.dcount);
    Serial.println("  playlist.listDir...");
    start = millis();
    uint32_t end3 = start;
    plist.files.clear();
    lcnt2.fcount = plist.listDir(rootDir, levels);
    end3 = millis() - start;
    Serial.printf("    listDir with SdFat and playlist takes %lu ms for %u files in %u /dirs\n", end3, plist.files.size(), plist.dirs.size()); 
    if ( printData ) {
        lcnt2.dcount = lcnt2.fcount = 0;
        for (auto e : plist.dirs) {
            Serial.printf("dirs[%03d] %s\n", ++lcnt2.dcount, e.path.c_str());
        }
        #if SAVE_FNAMES
        for (auto e : plist.files) {
            Serial.printf("files[%03d] %s\n", ++lcnt2.fcount, e.name.c_str());
        }
        #endif
    }
#endif
    Serial.println();
    createDir(SD, "/mydir");
    createDirWithOpen(SD, "/mydir/open.dir/");  // "dir" will be created as directiory ('/' at the end of path), works for SDFAT_FS.  
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
    
    //Delete the comment chars if needed:

    //testDir(SD, "/");       // shows timestamp lastWrite for all files in root.
    // Serial.printf("Total space: %luMB\n", SD.totalBytes() / (1024 * 1024));
    // Serial.printf("Used space: %luMB\n", SD.usedBytes() / (1024 * 1024));
    Serial.println("**** READY ****");
}

void loop() {}

//######################################################################

#if USE_SDFATFS
const char* _name(File32& f, const char *path) {
    static char m_path[512];
    int i = 0;
    if ( path ) {
        if ( (i = strlen(path)) < sizeof(m_path)-1 ) {
            strcpy(m_path, path);
            if (m_path[i-1] != '/') {
                m_path[i] = '/';
                if ( !(i == 1 && m_path[0] == '/') )    // not root (/) 
                    m_path[i++] = '/';
            }
        }
    }
    f.getName(m_path+i, sizeof(m_path)-i);
    return (const char*)m_path;
}

void listDir_SdFat(fs::FS &fs, const char * dirname, uint8_t levels, list_count_t& lc, bool list_hidden_files) {
    char path[256];
    int len = 0;
    File32 root;
    int mode = 2;

    strcpy(path, dirname); 
    len = strlen(path);
    if ( !(len == 1 && path[0] == '/') )    // not root (/) 
        path[len++] = '/';                  // without final \0

    root.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDir()) {
        Serial.println("Not a directory");
        return;
    }
    if (printData)
        Serial.printf("Scan directory(%d): %s\n", lc.dcount, dirname);
    lc.dcount++;
    while (true) {
        File32 file;
        file.openNext(&root, O_RDONLY);
        while (file) {
            if (file.isDir() && (!file.isHidden() || list_hidden_files)  && mode == 1) {
                if (levels) {
                    strcpy(path+len, _name(file));
                    listDir_SdFat(fs, path, levels - 1, lc, true);
                }
            }
            if (!file.isDir()  && (!file.isHidden() || list_hidden_files) && mode == 2) {
                lc.fcount++;
                #if SAVE_FNAMES
                    plist.files.emplace_back(0, 0, _name(file, dirname));
                #else
                    plist.files.emplace_back(0, file.dirIndex());
                #endif
                //Serial.printf("(%d)  FILE: %s\n", lc.fcount, _name(file, dirname));
                //Serial.printf("  SIZE: %d\n",file.size());
            }
            file.close();
            file.openNext(&root);
        }

        mode--;
        if (!mode) {
            root.close();
            break;
        }
        root.rewind();
    }
}
#endif

void listDir(fs::FS &fs, const char *dirname, uint8_t levels, list_count_t& lc, bool ignore_hidden_files) {
    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }
    if (printData)
        Serial.printf("Scan directory(%d): %s\n", lc.dcount, dirname);
    lc.dcount++;
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            //Serial.printf("  DIR : %s\n",file.name());
            if (levels) {
                listDir(fs, file.path(), levels - 1, lc, ignore_hidden_files);
            }
        }
        else {
            //Serial.printf("  FILE: %s\n",);

            lc.fcount++;
            #if USE_SDFATFS
            #if SAVE_FNAMES
                plist.files.emplace_back(0, 0, file.name());
            #else
                plist.files.emplace_back(0, 0);
            #endif 
            #endif
            //Serial.printf("(%d)  FILE: %s\n", lc.fcount, file.name());
            //Serial.printf("  SIZE: %d\n",file.size());
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
