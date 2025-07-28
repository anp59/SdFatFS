#include "sdfat_api.h"

oflag_t _convert_access_mode_to_flag(const char *mode) {
    static oflag_t modes[6] = {
        O_RDONLY,                       // r
        O_WRONLY | O_CREAT | O_TRUNC,   // w
        O_WRONLY | O_CREAT | O_APPEND,  // a
        O_RDWR,                         // r+
        O_RDWR | O_CREAT | O_TRUNC,     // w+
        O_RDWR | O_CREAT | O_APPEND,    // a+
    };
    int oflag_idx = 0;  // O_RONLY
    if ( mode ) {  // incorrect file modes are interpreted as READ. only '+' is recognised as an addition
        if (strlen(mode)) {
            switch (mode[0]) {
            case 'w':
                oflag_idx = 1;
                break;
            case 'a':
                oflag_idx = 2;
                break;
            }
            if (mode[1] == '+')
                oflag_idx += 3;
        }
    }
    return modes[oflag_idx];
}

using namespace fs;

// "create" true: creates missing folders in path if mode WRITE or APPEND and create the ../file as file or if it ends with '/' as Dir
// "create" false: create always a file and fails if the path not exists
// if mode  is READ.. "create" is ignored
FileImplPtr SDFATFSImpl::open(const char *path, const char *mode, const bool create = false) {
    if (create && (*mode == 'w' || *mode == 'a')) {
        PathAnalyze pa;
        if ( pa.analyze(path) ) { 
            _sdf.mkdir(pa.is_dir_path() ? pa.path() : pa.dir_name(), true); // no error check (mkdir fails if path exist)
            return std::make_shared<SDFATFSFileImpl>(this, path, !pa.is_dir_path() ? mode : "r"); 
        }
        return FileImplPtr();
    }
    return std::make_shared<SDFATFSFileImpl>(this, path, mode);
}

bool SDFATFSImpl::exists(const char *path) {
    return _sdf.exists(path);
}

bool SDFATFSImpl::rename(const char *pathFrom, const char *pathTo) {
    return _sdf.rename(pathFrom, pathTo);
}

bool SDFATFSImpl::remove(const char *path) {
    return _sdf.remove(path);
}

bool SDFATFSImpl::mkdir(const char *path) {
    return _sdf.mkdir(path);
}

bool SDFATFSImpl::rmdir(const char *path) {
    return _sdf.rmdir(path);
}

//***********************************************************************************

SDFATFSFileImpl::SDFATFSFileImpl(SDFATFSImpl *fs, const char *path, const char *mode) {
    if (_pa.analyze(path))
        _file = fs->_sdf.open(_pa.path(), _convert_access_mode_to_flag(mode));
};

SDFATFSFileImpl::SDFATFSFileImpl(FileType &file, const char *path) {
        _file.move(&file);
        _pa.analyze(path, getName(_file));
}

size_t SDFATFSFileImpl::write(const uint8_t *buf, size_t size) {
    return _file.write(buf, size);
}

// set return value to zero in case of SdFat read error (-1)
size_t SDFATFSFileImpl::read(uint8_t *buf, size_t size) {
    int rc = _file.read(buf, size);
    return ( rc == -1 ) ? 0 : (size_t)rc; 
}

void SDFATFSFileImpl::flush() {
    return _file.flush();
}

bool SDFATFSFileImpl::seek(uint32_t pos, SeekMode mode) {
    if (mode == fs::SeekMode::SeekSet) {
        return _file.seek(pos);
    }
    else if (mode == fs::SeekMode::SeekCur) {
        return _file.seek(position() + pos);
    }
    else if (mode == fs::SeekMode::SeekEnd) {
        return _file.seek(size() - pos);
    }
    return false;
}

size_t SDFATFSFileImpl::position() const {
	return ( !_file || _file.getError() ) ? (size_t)-1 : (size_t)_file.position();
}

size_t SDFATFSFileImpl::size() const {
    return (size_t)_file.size();
}

bool SDFATFSFileImpl::setBufferSize(size_t size) {
    return false;  // not possible for SdFat
}

void SDFATFSFileImpl::close() {

    _file.close();
}

const char *SDFATFSFileImpl::path() const {
    return _pa.path();
}

const char *SDFATFSFileImpl::name() const {
    //return getName(_file);    // If one asks the name of another file the same buffer will be used. So we assume here the name ptr is not kept.
    return _pa.base_name();
}

time_t SDFATFSFileImpl::getLastWrite() {
    uint16_t fat_date, fat_time;
    tm _tm;
        if (_file.getModifyDateTime(&fat_date, &fat_time)) {
		_tm.tm_sec = FS_SECOND(fat_time);
		_tm.tm_min = FS_MINUTE(fat_time);
		_tm.tm_hour = FS_HOUR(fat_time);
		_tm.tm_mday = FS_DAY(fat_date);
		_tm.tm_mon = FS_MONTH(fat_date) - 1;
		_tm.tm_year = FS_YEAR(fat_date) - 1900;
        _tm.tm_isdst = 1;   // set to 1 if DST status for the input datetime value was considered
        return mktime(&_tm);
    }
    return 0;
}

boolean SDFATFSFileImpl::isDirectory(void) {
    return _file.isDir();
}

boolean SDFATFSFileImpl::seekDir(long position) {
    return _file.seek((uint32_t)position);
}

String SDFATFSFileImpl::getNextFileName(void) {
    return getNextFileName(nullptr);
}

String SDFATFSFileImpl::getNextFileName(bool *isDir) {
    FileType f;
    if (_file.isDir()) {
        bool is_root = _file.isRoot();
        if (f.openNext(&_file)) {
            String name = _pa.path();
            if ( !is_root ) name += '/';
            name += getName(f);
            if (isDir) *isDir = f.isDir();
            f.close();
            return name;
        }
    }
    return "";
}

FileImplPtr SDFATFSFileImpl::openNextFile(const char *mode) {
    FileType &&f = _file.openNextFile(_convert_access_mode_to_flag(mode));
    return f ? std::make_shared<SDFATFSFileImpl>(f, _pa.path()) : FileImplPtr();    
}

void SDFATFSFileImpl::rewindDirectory(void) {
    return _file.rewind();
}
SDFATFSFileImpl::operator bool() {
    return _file.operator bool();
}

//******************************************************************
