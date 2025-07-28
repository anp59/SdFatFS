#include "Arduino.h"
#include "SdFat.h"
#include <vector>


/* 
* Internally, the directory names are always saved as a string. When using SdFat (SdFatFS), 
* the directory index (16 or 32-bit value) of the file can be saved (SAVE_FNAMES = 0) instead of the file name as a string (SAVE_FNAMES = 1).
* The complete path including the file name is then only generated and returned if required. 
* This can save considerable memory space and time when generating the list for many files. 
*/
#define SAVE_FNAMES 0

class SdFatPlayList {
    private:
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
    
    File32 cur_dir;
    File32 cur_dirfile;
    String cur_path;
    bool ignoreSpecialDirFiles;
    bool ignoreEmptyPaths;
    const char* name(File32& f, bool add_dirslash = false);
    int scan(const char *path, uint8_t levels);
      
    public:
        SdFatPlayList() : ignoreSpecialDirFiles(true), ignoreEmptyPaths(true) {}
        int create(const char *path, uint8_t levels = 0);
        const char* getFilePathAtIdx(size_t idx);
        size_t fileCount() {return files.size(); }
        size_t dirCount() {return dirs.size(); }
        void ignoreSystemAndHiddenFiles(bool val=true) { ignoreSpecialDirFiles = val; } 
        void ignoreEmptyDirPaths(bool val=true) { ignoreEmptyPaths = val; }
};

