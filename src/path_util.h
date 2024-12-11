#pragma once
#include <string.h>

/*
* analyze():    Analyze path and split it into into head (the directory component of path) 
*               and tail  (the non-directory component, that means the name of file or dir).
*               If parameter base is valid, base will be handled as basename and path as dirname. 
*               The argument is_dir_base can be used zu mark the base as directory instead of file.
*               Returns false, if there any errors (refer operator bool())
* path():       return analyzed path or nullptr if there are any errors
* dir_name():    return the directory portion of a pathname (head) without trailing slash
* base_name():   return non-directory portion of a pathname (tail)   
* is_dir_path():  set to true, if the arguments path or base ends with a slash (slash will be deleted in analyze())
* operator bool(): returns false, if there were any errors in in analyzing path (missing slash at start or failed memory allocation)
*/
class PathAnalyze
{
public:
    PathAnalyze() : temp(nullptr) {}

    bool analyze(const char* path, const char* base = "", bool isdirbase = false) {
        unsigned int i = 0;  // index in temp
        unsigned int baselen = 0;
        char c;
        init();
        if (path) {
            while (*path == ' ') path++;  // ltrim
            if (base) {
                while (*base == ' ' || *base == '/') base++; // ltrim
                baselen = strlen(base);
            }
            i = strlen(path);
            temp = (char*)malloc(i + baselen + 2);
        }
        if (temp) {
            strcpy(temp, path);
            if (*temp == '/') {
                while ( i > 1) {
                    c = temp[--i];
                    if (c == ' ') {
                        if (chr_readed)
                            chr_readed++;
                        else 
                            if (!dir_path)
                                temp[i] = '\0';  // rtrim
                        continue;
                    }
                    if (c == '/') {
                        if (chr_readed == 0) {
                            dir_path = true;
                            temp[i] = '\0';     // delete ending slash
                        }
                        else {
                            if ( chr_readed ) {
                                pos_slash = i;
                                break;
                            }
                        }
                    }
                    else {
                        if ( baselen ) break;
                        chr_readed++;
                    }
                }
                if (baselen > 0) {
                    if ( i > 1 ) {   // if path is root: i == 1, temp[i] == '\0'
                        strcat(temp, "/");
                        pos_slash = i + 1;
                    } 
                    strcat(temp, base);
                    while (base[baselen-1] == '/') { 
                        temp[strlen(temp)-1] = '\0';
                        baselen--;
                        isdirbase = true;
                    }
                    chr_readed = baselen;
                    dir_path = isdirbase;                 
                }
            }
            else log_e("%s does not start with /", path);
        }
        else log_e("malloc failed");
        return (status = ((temp != nullptr) && *temp));
    }

    ~PathAnalyze() {
        if (temp)
            free(temp);
    }

    const char* base_name() const {
        return (status) ? temp + pos_slash + 1 : nullptr; 
    } 
  
    const char* path() const {
        if (status) {
            if (pos_slash) temp[pos_slash] = '/'; 
            return temp;
        }
        return nullptr; 
    }

    const char* dir_name() const {
        if (status) {
            if (pos_slash) temp[pos_slash] = '\0';
            return pos_slash ? temp : "/";
        } 
        return nullptr;
    }

    bool is_dir_path() const { return dir_path; }

    operator bool() const {
        return status;
    }

private:
    void init() {
        chr_readed = 0;
        pos_slash = 0;
        dir_path = status = false;
        if (temp != nullptr)
            free(temp);
        temp = nullptr;
    }
    unsigned int chr_readed;    // for debugging purposes: length of basename. If (chr_readed==0 && pos_slash==0) path is the root directory.
    unsigned int pos_slash;     // position of slash between head and tail of path string.
                                // the length of the path can be expressed by (pos_slash+chr_readed+1). 
    char* temp;     // memory buffer for the path string
    bool dir_path;  // true, if parameter path ends with a slash
    bool status;
};
