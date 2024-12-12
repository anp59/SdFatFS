#include "Arduino.h"
#include "SdFat.h"
#include <vector>

// for testing purposes: set to 1 to save the filename in files 
// SAVE_FNAMES 0: createPlayList() with 1500 files in 64 dirs takes approx. 900 ms
// SAVE_FNAMES 1: createPlayList() with 1500 files in 64 dirs takes approx. 4700 ms
#define SAVE_FNAMES 0

int modulo(int a, int b); 

class fileFilter {
    std::vector<const char*> v_ext;
public:
        fileFilter() { init( {} ); }
        void init(const std::initializer_list<const char*> iLst) {
        v_ext.clear();
        v_ext.reserve(iLst.size());
        v_ext = iLst; 
    }

    bool operator()(File32& file) const;  
};

class SdFatPlayList {
    public:
    struct dir_t {
        dir_t(const char *p) : path(p) {}
        String path;
    };
    
    struct file_t {
        int dirs_index; 
        uint16_t fat_dir_index;
    #if SAVE_FNAMES
        String name;
        file_t(int di, uint16_t fdi, const char* n) : dirs_index(di), fat_dir_index(fdi), name(n) {}
    #else
        file_t(int di, uint16_t fdi) : dirs_index(di), fat_dir_index(fdi) {}
    #endif
    };
    

    std::vector<dir_t> dirs;
    std::vector<file_t> files;
    
    fileFilter is_valid_File;
    File32 cur_dir;
    File32 cur_dirfile;
    String cur_path;
    const char* name(File32& f, bool add_dirslash = false);

      
    
public:
    SdFatPlayList() {}
    void setFileFilter(const std::initializer_list<const char*> iLst_ext = { });
    int createPlayList(const char *path, uint8_t levels = 0);
    const char* getFileAtIdx(size_t idx); 
};

/*
--- Terminal on COM6 | 115200 8-N-1
--- Available filters and text transformations: colorize, debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time    
--- More details at https://bit.ly/pio-monitor-filters
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
[   445][I][esp32-hal-psram.c:96] psramInit(): PSRAM enabled
[  1343][W][main.cpp:56] setup(): read 64 dirs with 1457 files in 872 ms
info        PSRAM found, inputBufferSize: 638965 bytes
info        buffers freed, free Heap: 313036 bytes
info        Reading file: "/123_max raabe - das fräulein gerda.mp3"
info        MP3Decoder has been initialized, free Heap: 312876 bytes , free stack 6108 DWORDs 
....
*/