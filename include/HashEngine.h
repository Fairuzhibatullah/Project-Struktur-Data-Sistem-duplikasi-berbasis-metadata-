#ifndef HASH_ENGINE_H
#define HASH_ENGINE_H

#include "BaseEngine.h"
#include "Validator.h"
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>

class HashEngine : public BaseEngine {
private:
    std::unordered_map<std::string, DataArsip> databaseMap;
    std::vector<std::string> orderOfIDs;
    
    std::unordered_map<std::string, std::vector<std::string>> indexMetadata;
    std::unordered_map<std::string, std::vector<std::string>> indexNamaFile;
    std::unordered_map<std::string, std::vector<std::string>> indexKonten;

    int maxIDNum = 0;
    std::string datasetAsal = "";

    void tambahKeStruktur(const DataArsip& d) {
        databaseMap[d.id_dokumen] = d;
        orderOfIDs.push_back(d.id_dokumen);
        updateMaxIDNum(d.id_dokumen);

        std::string metadataKey = d.nama_file + "|" + std::to_string(d.ukuran_data);
        indexMetadata[metadataKey].push_back(d.id_dokumen);
        indexNamaFile[d.nama_file].push_back(d.id_dokumen);
        indexKonten[d.konten].push_back(d.id_dokumen);
    }

    void hapusDariStruktur(const std::string& id) {
        auto it = databaseMap.find(id);
        if (it != databaseMap.end()) {
            DataArsip d = it->second;
            
            // Hapus dari indeks metadata
            std::string metadataKey = d.nama_file + "|" + std::to_string(d.ukuran_data);
            auto& vMeta = indexMetadata[metadataKey];
            vMeta.erase(std::remove(vMeta.begin(), vMeta.end(), id), vMeta.end());
            if (vMeta.empty()) indexMetadata.erase(metadataKey);

            // Hapus dari indeks nama file
            auto& vNama = indexNamaFile[d.nama_file];
            vNama.erase(std::remove(vNama.begin(), vNama.end(), id), vNama.end());
            if (vNama.empty()) indexNamaFile.erase(d.nama_file);

            // Hapus dari indeks konten
            auto& vKonten = indexKonten[d.konten];
            vKonten.erase(std::remove(vKonten.begin(), vKonten.end(), id), vKonten.end());
            if (vKonten.empty()) indexKonten.erase(d.konten);

            // Hapus dari database utama
            databaseMap.erase(it);
        }
    }

public:
    HashEngine() {}

    int bacaCSV(const std::string& path) override {
        databaseMap.clear();
        orderOfIDs.clear();
        indexMetadata.clear();
        indexNamaFile.clear();
        indexKonten.clear();
        maxIDNum = 0;

        std::ifstream file(path);
        if (!file.is_open()) return -1;
        std::string baris;
        std::getline(file, baris); 
        int skipped = 0;
        while (std::getline(file, baris)) {
            if (baris.empty()) continue;
            std::vector<std::string> f = Validator::parseCSVLine(baris);
            if (f.size() < 6) { skipped++; continue; }
            DataArsip d;
            d.id_dokumen     = f[0];
            d.nama_file      = f[1];
            try { d.ukuran_data = std::stoll(f[2]); } catch (...) { d.ukuran_data = 0; }
            d.tanggal_unggah = f[3];
            d.sumber_data    = f[4];
            d.konten         = f[5];
            
            tambahKeStruktur(d);
        }
        file.close();
        return skipped;
    }

    void simpanKeFile(const std::string& path) override {
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cout << ">> ERROR: Gagal membuka file untuk disimpan.\n";
            return;
        }
        file << "id_dokumen,nama_file,ukuran_data,tanggal_unggah,sumber_data,konten\n";
        
        std::vector<std::string> cleanOrder;
        for (const auto& id : orderOfIDs) {
            auto it = databaseMap.find(id);
            if (it != databaseMap.end()) {
                const auto& d = it->second;
                std::string kontenSafe = d.konten;
                if (kontenSafe.find(',') != std::string::npos)
                    kontenSafe = "\"" + kontenSafe + "\"";
                file << d.id_dokumen     << ","
                     << d.nama_file      << ","
                     << d.ukuran_data    << ","
                     << d.tanggal_unggah << ","
                     << d.sumber_data    << ","
                     << kontenSafe       << "\n";
                cleanOrder.push_back(id);
            }
        }
        orderOfIDs = cleanOrder;
        file.close();
    }

    std::string getDatasetAsal() const override { return datasetAsal; }
    void setDatasetAsal(const std::string& asal) override { datasetAsal = asal; }

    std::string generateNextID() override {
        maxIDNum++;
        std::ostringstream oss;
        oss << "DOC-" << std::setw(7) << std::setfill('0') << maxIDNum;
        return oss.str();
    }

    void updateMaxIDNum(const std::string& id) override {
        size_t dash = id.rfind('-');
        if (dash != std::string::npos) {
            try {
                int num = std::stoi(id.substr(dash + 1));
                if (num > maxIDNum) maxIDNum = num;
            } catch (...) {}
        }
    }

    void resetMaxIDNum() override { maxIDNum = 0; }

    std::string cariDuplikatMetadata(const std::string& nama, long long ukuran) override {
        std::string key = nama + "|" + std::to_string(ukuran);
        auto it = indexMetadata.find(key);
        if (it != indexMetadata.end() && !it->second.empty()) {
            return it->second[0];
        }
        return "";
    }

    std::string cariDuplikatKonten(const std::string& konten) override {
        auto it = indexKonten.find(konten);
        if (it != indexKonten.end() && !it->second.empty()) {
            return it->second[0];
        }
        return "";
    }

    void insertRecord(const DataArsip& d) override {
        tambahKeStruktur(d);
    }

    bool searchRecord(const std::string& query, std::vector<DataArsip>& results) override {
        results.clear();
        auto it = databaseMap.find(query);
        if (it != databaseMap.end()) {
            results.push_back(it->second);
            return true;
        }
        
        auto itNama = indexNamaFile.find(query);
        if (itNama != indexNamaFile.end()) {
            for (const auto& id : itNama->second) {
                auto itDB = databaseMap.find(id);
                if (itDB != databaseMap.end()) {
                    results.push_back(itDB->second);
                }
            }
        }
        return !results.empty();
    }

    bool getRecordByID(const std::string& id, DataArsip& d) override {
        auto it = databaseMap.find(id);
        if (it != databaseMap.end()) {
            d = it->second;
            return true;
        }
        return false;
    }

    bool updateMetadata(const std::string& id, const std::string& namaBaru, long long ukuranBaru, const std::string& kontenBaru) override {
        auto it = databaseMap.find(id);
        if (it != databaseMap.end()) {
            DataArsip& d = it->second;
            
            // Hapus indeks metadata lama
            std::string oldMetadataKey = d.nama_file + "|" + std::to_string(d.ukuran_data);
            auto& vMeta = indexMetadata[oldMetadataKey];
            vMeta.erase(std::remove(vMeta.begin(), vMeta.end(), id), vMeta.end());
            if (vMeta.empty()) indexMetadata.erase(oldMetadataKey);

            // Hapus indeks nama file lama
            auto& vNama = indexNamaFile[d.nama_file];
            vNama.erase(std::remove(vNama.begin(), vNama.end(), id), vNama.end());
            if (vNama.empty()) indexNamaFile.erase(d.nama_file);

            // Hapus indeks konten lama
            auto& vKonten = indexKonten[d.konten];
            vKonten.erase(std::remove(vKonten.begin(), vKonten.end(), id), vKonten.end());
            if (vKonten.empty()) indexKonten.erase(d.konten);

            // Update
            d.nama_file = namaBaru;
            d.ukuran_data = ukuranBaru;
            d.konten = kontenBaru;

            // Re-index
            std::string newMetadataKey = namaBaru + "|" + std::to_string(ukuranBaru);
            indexMetadata[newMetadataKey].push_back(id);
            indexNamaFile[namaBaru].push_back(id);
            indexKonten[kontenBaru].push_back(id);
            return true;
        }
        return false;
    }

    bool deleteRecord(const std::string& id) override {
        auto it = databaseMap.find(id);
        if (it != databaseMap.end()) {
            hapusDariStruktur(id);
            return true;
        }
        return false;
    }

    void clearAll() override {
        databaseMap.clear();
        orderOfIDs.clear();
        indexMetadata.clear();
        indexNamaFile.clear();
        indexKonten.clear();
        maxIDNum = 0;
        datasetAsal = "";
    }

    void scanDuplicates(int mode) override {
        if (mode != 1 && mode != 2) return;
        
        auto start = std::chrono::steady_clock::now();
        int grupDuplikat        = 0;
        int totalRecordDuplikat = 0;

        const auto& targetMap = (mode == 1) ? indexMetadata : indexKonten;

        for (const auto& pair : targetMap) {
            if (pair.second.size() > 1) {
                grupDuplikat++;
                
                if (mode == 1) {
                    std::string key = pair.first;
                    size_t pipe = key.rfind('|');
                    std::string nama = key.substr(0, pipe);
                    std::string ukuran = key.substr(pipe + 1);
                    std::cout << "\n[Grup " << grupDuplikat << "] "
                              << nama << " | " << ukuran << " bytes"
                              << " (" << pair.second.size() << " salinan)\n";
                } else {
                    std::cout << "\n[Grup " << grupDuplikat << "] Konten: "
                              << pair.first
                              << " (" << pair.second.size() << " salinan)\n";
                }

                for (const auto& id : pair.second) {
                    auto it = databaseMap.find(id);
                    if (it != databaseMap.end()) {
                        const auto& d = it->second;
                        std::cout << "  -> ID: " << d.id_dokumen
                                  << " | File: " << d.nama_file
                                  << " | Ukuran: " << d.ukuran_data << " bytes"
                                  << " | Sumber: " << d.sumber_data << "\n";
                        totalRecordDuplikat++;
                    }
                }
            }
        }

        auto stop = std::chrono::steady_clock::now();
        auto dur  = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        if (grupDuplikat == 0)
            std::cout << ">> Tidak ada duplikat ditemukan.\n";
        else
            std::cout << "\n>> " << grupDuplikat << " grup duplikat ("
                      << totalRecordDuplikat << " record terlibat).\n";
        std::cout << ">> Waktu pemindaian (Hash Scan): " << dur.count() << " ms.\n";
    }

    void dapatkanStatistik(int& total, int& unik, int& grupDup, int& recordDup, double& waktuMs) override {
        auto start = std::chrono::steady_clock::now();

        total = (int)databaseMap.size();
        grupDup        = 0;
        recordDup = 0;

        for (const auto& pair : indexMetadata) {
            if (pair.second.size() > 1) {
                grupDup++;
                recordDup += (int)pair.second.size();
            }
        }

        unik = total - recordDup;

        auto stop = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> dur = stop - start;
        waktuMs = dur.count();
    }
};

#endif // HASH_ENGINE_H
