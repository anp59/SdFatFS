#pragma once

#include "FS.h"
#include "SdFat.h"
#include "sd_defines.h"

namespace fs {
class SDFATFS : public FS
{
  public:
    SDFATFS(FSImplPtr impl);
    bool begin(SdCsPin_t csPin = SS, uint32_t maxSck = SD_SCK_MHZ(25));
    bool begin(SdSpiConfig spiConfig);
    void end();

    sdcard_type_t cardType();
    uint64_t cardSize() { return totalBytes(); }
    size_t numSectors();
    size_t sectorSize();
    uint64_t totalBytes();  // return (uint64_t)clusterCount() * (uint64_t)bytesPerCluster();
    uint64_t usedBytes();   // return (uint64_t)(clusterCount() - freeClusterCount()) * (uint64_t)bytesPerCluster();
    bool readRAW(uint8_t *buffer, uint32_t sector);
    bool writeRAW(uint8_t *buffer, uint32_t sector);
    SdFat &operator()();  // returns the internal SdFat-object - allow access, so users can mix SDFATFS & SdFat APIs
};
}  // namespace fs

extern fs::SDFATFS SDF;

using namespace fs;
typedef fs::File SDFATFile;
typedef fs::SDFATFS SDFATFileSystemClass;
#define SDFATFSSystem SDF
