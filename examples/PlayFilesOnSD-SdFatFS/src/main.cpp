/*
    In this example, a playlist of all (audio) files with the specified file extensions is created from an SD card 
    and their titles are played back via the ESP32-audioI2S library (https://github.com/schreibfaul1/ESP32-audioI2S.git). 
    Unlike most implementations, the SdFatFS library (https://github.com/anp59/SdFatFS.git) is used here instead of 
    the SD or SD_MMC libs. 

    The SdFatFS library implements the functions of the Arduino FS interface based on the SdFat solution from Greimann 
    (https://github.com/greiman/SdFat) and can therefore be used as an alternative to the SD or SD_MMC libraries 
    in order to use additional SdFat functionalities in projects. 

    Using the SdFat library allows you to use only the so-called directory index instead of the file names 
    of the files (short int for FAT32). This allows the time required to create of the playlist 
    can be considerably shortened: Read 80 dirs with 1774 files in 617 ms (tested with 50 MHz SPI_CLK) 
    The internal structure of the playlist also allows each directory path to be saved only once.

    The terminal can be used to navigate through the playlist and control the playback volume.
    The control commands via terminal are:
        Space bar -> next song
        Enter/Return key -> repeat current song
        Entering a decimal number -> set offset to next song (positive value: forwards - negative value: backwards)
        '<' Volume down
        '>' Volume up

    Note: As when using the SD library, the SD card must be operated with SdFatFS via SPI. 
    When using the YB-ESP32-S3-AMP board from yellobyte 
    (https://github.com/yellobyte/ESP32-DevBoards-Getting-Started/tree/main/boards/YB-ESP32-S3-AMP)
    the solder bridge SD_CS must be closed [default].

    Last updated 2025-07-26, anp59
    Example was tested with the last lib versions
*/

#include <Arduino.h>
#include "SD_SDFAT.h"
#include "Audio.h"  // Audio.h should included after SD_SDFAT to avoid compiler warnings
#include "SdFatPlayList.h"

#if CONFIG_IDF_TARGET_ESP32S3
    #define I2S_BCLK    5   // YB-ESP32-S3-AMP: GPIOs 5/6/7 are not wired to a pin, they are exclusively used for the MAX98357A
    #define I2S_LRC     6
    #define I2S_DOUT    7
    SPIClass SD_SPI(FSPI);  // GPIOs: MOSI=11, CLK=12, MISO=13, CS=10
#endif

#if CONFIG_IDF_TARGET_ESP32
    #define I2S_LRC     26
    #define I2S_DOUT    25
    #define I2S_BCLK    27
    SPIClass SD_SPI(VSPI);  // GPIOs: MOSI=23, CLK=18, MISO=19, CS=5
#endif

Audio audio;
SdFatPlayList plist;
bool playNextFile(int offset = 1);
bool f_eof = true;
const uint8_t volume_steps = 21;
const uint8_t default_volume = 3;

const char *dir = "/";      // root dir for the playlist
int subdirLevels = 10;      // subdirLevels = 0 : add only files from dir to playlist. 
                            // subdirLevels > 0 : add files in dir files from all subdirs down to a depth of subdirLevels to the lis 

/**************************************************/

// new with V3.4.2
void my_audio_info(Audio::msg_t m) {
    Serial.printf("%s: %s\n", m.s, m.msg);
    if ( m.e == Audio::evt_eof ) f_eof = true;
}

/**************************************************/
void setup() {
    Serial.begin(115200);
    // SPI CLK 50 MHZ was successfully tested with the YB-ESP32-S3 AMP board from yellobyte. If problems occur, the CLK frequency should be reduced to 25 or 16 MHz.
    if ( !SDF.begin(SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(50), &SD_SPI)) ) {
        log_e("Card Mount failed!");
        return;
    }
    Serial.println(audio.getVersion());
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolumeSteps(volume_steps);
    audio.setVolume(default_volume); // 0...21 Will need to add a volume setting in the app

    // new with V3.4.2
    Audio::audio_info_callback = my_audio_info;
    
    plist.setFileFilter( {"mp3", "ogg", "wav"} );   // optional list consist of the  extensions of files to be considered for the playlist. Empty list = all file types 
    uint32_t start = millis();
    uint32_t end = start;
    plist.createPlayList(dir, subdirLevels);
    end = millis()-start;
        
    if (plist.files.empty()) {
        log_e("No files in playlist!");
        f_eof = false;
        return;
    }
    //Serial.printf(
    log_i(
"\n\n\
Read %d dirs with %d files in %lu ms.\n\
Playlist navigation:\n\
    Space -> next song\n\
    Enter -> repeat current song\n\
    Decimal number -> offset to next song (positive value: forwards - negative value: backwards)\n\
    '<' Volume down\n\
    '>' Volume up\n", plist.dirs.size(), plist.files.size(), end );
    
    f_eof = !playNextFile(0);   // play first file of the playlist (index 0)   
}

void loop() {
    int offset = 1;    
    char c = 0;
    static int cur_volume = default_volume;
    
    audio.loop();

    // control next song
    if (Serial.available()) {
        c = Serial.read();
        if ( !(c == '<' || c == '>' || c == ' ') ) {
            String s(c);
            s += Serial.readString();
                offset = s.toInt();
        }                       
        if ( !(c == '<' || c == '>') ) {    
            audio.stopSong();
            f_eof = true;
        }
    }
    if (f_eof) {
        f_eof = !playNextFile(offset);
        vTaskDelay(150);
    }

    // volume control
    if  ( c == '<' || c == '>') { 
        cur_volume += (c == '>' ? 1 : -1);
        if (cur_volume < 0) cur_volume = 0;
        if (cur_volume > volume_steps) cur_volume = volume_steps; 
        audio.setVolume(cur_volume);
        log_i("Volume = %d", cur_volume);
    }
    vTaskDelay(1);
}

/****************************************************/

bool playNextFile(int offset) {
    static int cur_pos = 0;
    const char *file_path;
    if ( plist.files.size() ) {
        cur_pos = modulo(cur_pos += offset, plist.files.size());
        if ( (file_path = plist.getFilePathAtIdx(cur_pos)) != nullptr ) {
            if ( audio.connecttoFS(SDF, file_path) ) {
                Serial.printf("\n**** now playing at [%d]: %s\n", cur_pos, file_path);
                return true;
            }
            else
                log_e("connectToSD failed: %s\n", file_path);
        }
    }
    return false;
}


// obsolet with V3.4.2
// optional
void audio_info(const char *info) {
    Serial.print("info        "); Serial.println(info);
}
// void audio_id3data(const char *info) {  //id3 metadata
//     Serial.print("id3data     ");Serial.println(info);
// }
void audio_eof_mp3(const char *info) {  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
    f_eof = true;
}
