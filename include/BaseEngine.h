#ifndef BASE_ENGINE_H
#define BASE_ENGINE_H

#include "DataArsip.h"
#include <string>
#include <vector>

class BaseEngine {
public:
    virtual ~BaseEngine() {}
    
    // Core file management
    virtual int bacaCSV(const std::string& path) = 0;
    virtual void simpanKeFile(const std::string& path) = 0;
    
    // Engine metadata
    virtual std::string getDatasetAsal() const = 0;
    virtual void setDatasetAsal(const std::string& asal) = 0;
    
    // ID generation
    virtual std::string generateNextID() = 0;
    virtual void updateMaxIDNum(const std::string& id) = 0;
    virtual void resetMaxIDNum() = 0;
    
    // Query / Duplication checking
    virtual std::string cariDuplikatMetadata(const std::string& nama, long long ukuran) = 0;
    virtual std::string cariDuplikatKonten(const std::string& konten) = 0;
    
    // CRUD Operations
    virtual void insertRecord(const DataArsip& d) = 0;
    virtual bool searchRecord(const std::string& query, std::vector<DataArsip>& results) = 0;
    virtual bool getRecordByID(const std::string& id, DataArsip& d) = 0;
    virtual bool updateMetadata(const std::string& id, const std::string& namaBaru, long long ukuranBaru, const std::string& kontenBaru) = 0;
    virtual bool deleteRecord(const std::string& id) = 0;
    virtual void clearAll() = 0;
    
    // Advanced duplication scanning and statistics
    virtual void scanDuplicates(int mode) = 0; // mode 1: metadata, mode 2: konten
    virtual void dapatkanStatistik(int& total, int& unik, int& grupDup, int& recordDup, double& waktuMs) = 0;
};

#endif
