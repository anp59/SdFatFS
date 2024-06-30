#include <Arduino.h>
#include "SD_SDFAT.h"
#include "Audio.h"

#include <vector>

#define I2S_LRC     26
#define I2S_DOUT    25
#define I2S_BCLK    27

Audio audio;
std::vector<char*>  v_audioContent;
bool play_next = true;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels); 
void vector_clear_and_shrink(vector<char*>&vec);
bool playNext();
File dir;
const char audioDir[] = "/";  // directory to play


void setup() {
    Serial.begin(115200);
    if ( !SDF.begin() ) {
        Serial.println("Card Mount Failed");
        return;
    }

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(7); // 0...21 Will need to add a volume setting in the app
    dir = SDF.open(audioDir);
    listDir(SDF, audioDir, 1);
}


void loop() {
    if (Serial.available()) {
        Serial.readString(); 
        audio.stopSong();
        play_next = !playNext();
    }
    if (play_next) {
        play_next = !playNext();
    }
    audio.loop();
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.println(file.name());
            v_audioContent.insert(v_audioContent.begin(), strdup(file.path()));
        }
        file = root.openNextFile();
    }
    Serial.printf("num files %i\n", v_audioContent.size());
    root.close();
    file.close();
}

void vector_clear_and_shrink(vector<char*>&vec){
    uint size = vec.size();
    for (int i = 0; i < size; i++) {
        if(vec[i]){
            free(vec[i]);
            vec[i] = NULL;
        }
    }
    vec.clear();
    vec.shrink_to_fit();
}

bool playNext() {
    bool rc = false;
    if(v_audioContent.size() > 0) {
        const char* s = (const char*)v_audioContent[v_audioContent.size() -1];
        Serial.printf("playing %s\n", s);
        play_next = false;
        rc = audio.connecttoFS(SDF, s);
        v_audioContent.pop_back();
    }
    return rc;
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
    if(v_audioContent.size() == 0){
        vector_clear_and_shrink(v_audioContent); // free memory
        return;
    }
    play_next = true;
}
