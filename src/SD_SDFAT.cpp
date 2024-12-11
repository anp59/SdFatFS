#include "SD_SDFAT.h"
#include "sdfat_api.h"

using namespace fs;

static SdFat sd_obj;

SDFATFS::SDFATFS(FSImplPtr impl) : FS(impl) {}

bool SDFATFS::begin(SdCsPin_t csPin, uint32_t maxSck) { 
    return sd_obj.begin(csPin, maxSck);
}

bool SDFATFS::begin(SdSpiConfig spiConfig) {
    return sd_obj.begin(spiConfig);
}

void SDFATFS::end() { return sd_obj.end(); }

sdcard_type_t SDFATFS::cardType() {
    if (! sd_obj.sdErrorCode() ) {    // SD mount failed = 43
        uint8_t type = sd_obj.card()->type();
        if ( type == SD_CARD_TYPE_SD1) return CARD_SD;
        if ( type == SD_CARD_TYPE_SD2) return CARD_SD;
        if ( type == SD_CARD_TYPE_SDHC) return CARD_SDHC;
        if ( type != 0) return CARD_UNKNOWN;
    }
    return CARD_NONE;
}

size_t SDFATFS::numSectors() {
    return (sd_obj.sectorsPerCluster() * sd_obj.clusterCount());
}
size_t SDFATFS::sectorSize() {
    return (sd_obj.bytesPerCluster() / sd_obj.sectorsPerCluster());
}
    
uint64_t SDFATFS::totalBytes() { 
    return (uint64_t)sd_obj.clusterCount() * (uint64_t)sd_obj.bytesPerCluster(); 
}

uint64_t SDFATFS::usedBytes() {
    return (uint64_t)(sd_obj.clusterCount() - sd_obj.freeClusterCount()) * (uint64_t)sd_obj.bytesPerCluster();
}   

bool SDFATFS::readRAW(uint8_t* buffer, uint32_t sector) {
    return sd_obj.card()->readSector(sector, buffer);
}
bool SDFATFS::writeRAW(uint8_t* buffer, uint32_t sector) {
    return sd_obj.card()->writeSector(sector, buffer);
}

SdFat& SDFATFS::operator()() {
    return sd_obj;
}

    
SDFATFS SDF = SDFATFS(FSImplPtr(new SDFATFSImpl(sd_obj)));
