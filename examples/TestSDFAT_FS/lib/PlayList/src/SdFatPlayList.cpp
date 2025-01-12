#include "SdFatPlayList.h"

const char* SdFatPlayList::name(File32& f, bool add_dirslash) {
    static char m_path[256+1];
    int i = 0;
    if ( add_dirslash ) 
        m_path[i++] = '/';
    m_path[i] = '\0';
    f.getName(m_path+i, sizeof(m_path)-i);
    return (const char*)m_path;
}

int SdFatPlayList::listDir(const char *path, uint8_t levels) {    
    std::vector<String> sub_dirs;
    String sub_dir_path;
    int dirs_index = dirs.size();
    //file_count = 0;   // count files of dir
    
    if (!cur_dir.open(path)) {
        log_e("%s open failed", path);
        return 0;
    }
    while (cur_dir) {
        if ( cur_dirfile.openNext(&cur_dir) ) {
            //if ( !(cur_dirfile.isSystem() || cur_dirfile.isHidden()) ) {    // hidden: filename starts with \.
                if (cur_dirfile.isDir()) {
                    if (levels) {
                        sub_dirs.emplace_back(name(cur_dirfile, !cur_dir.isRoot()));
                        log_d("subdirs.push: levels: %d, path: %s", levels, sub_dirs.back().c_str());
                    }
                }
                else {
                    //if ( is_valid_File(cur_dirfile) )  {
                        #if SAVE_FNAMES
                            files.emplace_back(dirs_index, cur_dirfile.dirIndex(), name(cur_dirfile)); 
                        #else
                            files.push_back({dirs_index, cur_dirfile.dirIndex()}); 
                        #endif
                        //file_count++;
                    //}
                }
            //}
            cur_dirfile.close();
        }
        else 
            break;
    }
    cur_dir.close();
    
    dirs.emplace_back(path);
    if (levels) {
        for ( auto e : sub_dirs ) {
            (sub_dir_path = path) += e;
            log_d("call listDir: levels: %d, path: = %s, dirs_size = %d, files: %d", levels, sub_dir_path.c_str(), dirs.size(), files.size());            
            listDir(sub_dir_path.c_str(), levels-1);
        }
    }
    return files.size();
}
