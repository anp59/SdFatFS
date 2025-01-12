#include "Arduino.h"
#include "SdFat.h"
#include <vector>

#define SAVE_FNAMES 1

class SdFatPlayList {
    public:
    struct dir_t {
        //dir_t(const char *p) : path(std::move(p)) {}
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
    
    File32 cur_dir;
    File32 cur_dirfile;
    const char* name(File32& f, bool add_dirslash = false);

      
    
    public:
    SdFatPlayList() {}
    int listDir(const char *path, uint8_t levels = 0);
};

