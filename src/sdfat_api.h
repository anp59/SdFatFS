#pragma once

#define DISABLE_FS_H_WARNING
#include "FS.h"
#include "FSImpl.h"
#include "SdFat.h"
#include "path_util.h"

using namespace fs;

class FilePath;
class SDFATFSFileImpl;

class SDFATFSImpl : public FSImpl
{
  protected:
    friend class SDFATFSFileImpl;
    SdFat &_sdf;

  public:
    SDFATFSImpl(SdFat &sdf) : _sdf(sdf) {}
    FileImplPtr open(const char *path, const char *mode, const bool create) override; // ToDo: create=true erzeugt Dateien inkl. der notwendigen Verzeichnisse, wenn mode w oder a
    bool exists(const char *path) override;
    bool rename(const char *pathFrom, const char *pathTo) override;
    bool remove(const char *path) override;
    bool mkdir(const char *path) override;
    bool rmdir(const char *path) override;
};

// SdFat version 2.2.3 or greater required!
class SDFATFSFileImpl : public FileImpl
{
  protected:
#if SDFAT_FILE_TYPE < 2
    /** Select type for File. */
    typedef File32 FileType;
#elif SDFAT_FILE_TYPE == 2
    typedef ExFile FileType;
#elif SDFAT_FILE_TYPE == 3
    typedef FsFile FileType;
#endif  // SDFAT_FILE_TYPE

    mutable FileType _file;
    PathAnalyze _pa;
    const char *getName( FileType &file) const {
        static char _buf[256];
        file.getName(_buf, sizeof(_buf));
        return _buf;
    }

  public:
    SDFATFSFileImpl(SDFATFSImpl *fs, const char *path, const char *mode);  
    SDFATFSFileImpl(FileType &file, const char *path);
    ~SDFATFSFileImpl() { _file.close(); };
    size_t write(const uint8_t *buf, size_t size) override;
    size_t read(uint8_t *buf, size_t size) override;
    void flush() override;
    bool seek(uint32_t pos, SeekMode mode) override;
    size_t position() const override;
    size_t size() const override;
    bool setBufferSize(size_t size);
    void close() override;
    const char *path() const override;
    const char *name() const override;
    time_t getLastWrite() override;
    boolean isDirectory(void) override;
    boolean seekDir(long position) override;
    String getNextFileName(void) override;
    String getNextFileName(bool *isDir) override;
    FileImplPtr openNextFile(const char *mode) override;
    void rewindDirectory(void) override;
    operator bool();
};
