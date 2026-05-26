#ifndef VECTOR_ENGINE_H
#define VECTOR_ENGINE_H

#include "BaseEngine.h"
#include "Validator.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>

class VectorEngine : public BaseEngine {
private:
    std::vector<DataArsip> database;
    int maxIDNum = 0;
    std::string datasetAsal = "";

public:
    VectorEngine() {}

    int bacaCSV(const std::string& path) override {
        database.clear();
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
            
            database.push_back(d);
            updateMaxIDNum(d.id_dokumen);
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
        for (const auto& d : database) {
            std::string kontenSafe = d.konten;
            if (kontenSafe.find(',') != std::string::npos)
                kontenSafe = "\"" + kontenSafe + "\"";
            file << d.id_dokumen     << ","
                 << d.nama_file      << ","
                 << d.ukuran_data    << ","
                 << d.tanggal_unggah << ","
                 << d.sumber_data    << ","
                 << kontenSafe       << "\n";
        }
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
        for (const auto& d : database) {
            if (d.nama_file == nama && d.ukuran_data == ukuran) {
                return d.id_dokumen;
            }
        }
        return "";
    }

    std::string cariDuplikatKonten(const std::string& konten) override {
        for (const auto& d : database) {
            if (d.konten == konten) {
                return d.id_dokumen;
            }
        }
        return "";
    }

    void insertRecord(const DataArsip& d) override {
        database.push_back(d);
        updateMaxIDNum(d.id_dokumen);
    }

    bool searchRecord(const std::string& query, std::vector<DataArsip>& results) override {
        results.clear();
        for (const auto& d : database) {
            if (d.id_dokumen == query || d.nama_file == query) {
                results.push_back(d);
            }
        }
        return !results.empty();
    }

    bool getRecordByID(const std::string& id, DataArsip& d) override {
        for (const auto& record : database) {
            if (record.id_dokumen == id) {
                d = record;
                return true;
            }
        }
        return false;
    }

    bool updateMetadata(const std::string& id, const std::string& namaBaru, long long ukuranBaru, const std::string& kontenBaru) override {
        for (auto& d : database) {
            if (d.id_dokumen == id) {
                d.nama_file = namaBaru;
                d.ukuran_data = ukuranBaru;
                d.konten = kontenBaru;
                return true;
            }
        }
        return false;
    }

    bool deleteRecord(const std::string& id) override {
        for (auto it = database.begin(); it != database.end(); ++it) {
            if (it->id_dokumen == id) {
                database.erase(it);
                return true;
            }
        }
        return false;
    }

    void clearAll() override {
        database.clear();
        maxIDNum = 0;
        datasetAsal = "";
    }

    void scanDuplicates(int mode) override {
        if (mode != 1 && mode != 2) return;
        
        auto start = std::chrono::steady_clock::now();
        int total = (int)database.size();
        std::vector<bool> sudahDicetak(total, false);
        int grupDuplikat        = 0;
        int totalRecordDuplikat = 0;

        for (int i = 0; i < total; i++) {
            if (sudahDicetak[i]) continue;
            std::vector<int> grup = {i};
            for (int j = i + 1; j < total; j++) {
                if (!sudahDicetak[j]) {
                    bool isMatch = false;
                    if (mode == 1) {
                        isMatch = (database[i].nama_file == database[j].nama_file &&
                                   database[i].ukuran_data == database[j].ukuran_data);
                    } else {
                        isMatch = (database[i].konten == database[j].konten);
                    }
                    if (isMatch) grup.push_back(j);
                }
            }
            if (grup.size() > 1) {
                grupDuplikat++;
                if (mode == 1) {
                    std::cout << "\n[Grup " << grupDuplikat << "] "
                              << database[i].nama_file << " | "
                              << database[i].ukuran_data << " bytes"
                              << " (" << grup.size() << " salinan)\n";
                } else {
                    std::cout << "\n[Grup " << grupDuplikat << "] Konten: "
                              << database[i].konten
                              << " (" << grup.size() << " salinan)\n";
                }
                
                for (int idx : grup) {
                    std::cout << "  -> ID: " << database[idx].id_dokumen
                              << " | File: " << database[idx].nama_file
                              << " | Ukuran: " << database[idx].ukuran_data << " bytes"
                              << " | Sumber: " << database[idx].sumber_data << "\n";
                    sudahDicetak[idx] = true;
                    totalRecordDuplikat++;
                }
            } else {
                sudahDicetak[i] = true;
            }
        }

        auto stop = std::chrono::steady_clock::now();
        auto dur  = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        if (grupDuplikat == 0)
            std::cout << ">> Tidak ada duplikat ditemukan.\n";
        else
            std::cout << "\n>> " << grupDuplikat << " grup duplikat ("
                      << totalRecordDuplikat << " record terlibat).\n";
        std::cout << ">> Waktu pemindaian (Vector Scan): " << dur.count() << " ms.\n";
    }

    void dapatkanStatistik(int& total, int& unik, int& grupDup, int& recordDup, double& waktuMs) override {
        auto start = std::chrono::steady_clock::now();

        total = (int)database.size();
        std::vector<bool> sudahDihitung(total, false);
        grupDup        = 0;
        recordDup = 0;

        for (int i = 0; i < total; i++) {
            if (sudahDihitung[i]) continue;
            std::vector<int> grup = {i};
            for (int j = i + 1; j < total; j++) {
                if (!sudahDihitung[j] &&
                    database[i].nama_file   == database[j].nama_file &&
                    database[i].ukuran_data == database[j].ukuran_data)
                    grup.push_back(j);
            }
            if (grup.size() > 1) {
                grupDup++;
                recordDup += (int)grup.size();
            }
            for (int idx : grup) sudahDihitung[idx] = true;
        }

        unik = total - recordDup;

        auto stop = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> dur = stop - start;
        waktuMs = dur.count();
    }
};

#endif // VECTOR_ENGINE_H
