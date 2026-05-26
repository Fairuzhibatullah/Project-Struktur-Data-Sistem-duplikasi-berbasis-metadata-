#ifndef ARSIP_MANAGER_H
#define ARSIP_MANAGER_H

#include "BaseEngine.h"
#include "Validator.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <limits>

class ArsipManager {
private:
    BaseEngine* engine;
    std::string namaStruktur;
    std::string fileDatabase;
    std::string statsLogFile;

public:
    ArsipManager(BaseEngine* eng, const std::string& namaStr, const std::string& fileDB, const std::string& statsLog = "statistik_log.csv")
        : engine(eng), namaStruktur(namaStr), fileDatabase(fileDB), statsLogFile(statsLog) {}

    void runMenu() {
        std::cout << "============================================\n";
        std::cout << "  SISTEM DETEKSI DUPLIKASI ARSIP ( " << namaStruktur << " )  \n";
        std::cout << "  Deteksi berbasis: nama_file + ukuran_data / konten\n";
        std::cout << "============================================\n";

        std::ifstream cekDB(fileDatabase);
        bool dbAda = cekDB.is_open();
        cekDB.close();

        if (dbAda) {
            std::cout << "\nDatabase ditemukan: " << fileDatabase << "\n";
            std::cout << "1. Lanjut dari database yang ada\n";
            std::cout << "2. Import dataset baru (database lama akan diganti)\n";
            std::cout << "Pilih (1-2): ";
            int pilihan; std::cin >> pilihan;

            if (pilihan == 1) {
                auto start = std::chrono::steady_clock::now();
                int skipped = engine->bacaCSV(fileDatabase);
                auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
                
                if (skipped == -1) {
                    std::cout << "   [!] Database kosong, import dataset baru.\n";
                    dbAda = false;
                } else {
                    engine->setDatasetAsal(fileDatabase);
                    
                    int total, unik, grupDup, recordDup;
                    double waktu;
                    engine->dapatkanStatistik(total, unik, grupDup, recordDup, waktu);
                    std::cout << ">> Berhasil memuat " << total << " data dalam " << dur.count() << " ms.\n";
                }
            } else {
                dbAda = false;
            }
        }

        if (!dbAda) {
            while (true) {
                std::string sumber = pilihFile("Import Dataset");
                if (muatDariDataset(sumber)) break;
                std::cout << "   [!] File tidak ditemukan atau kosong, silakan pilih ulang.\n";
            }
        }

        int menu;
        do {
            std::cout << "\n--- MENU UTAMA ---\n"
                      << "1. Insert Manual\n"
                      << "2. Batch Import (CSV)\n"
                      << "3. Cari Data (Search)\n"
                      << "4. Tampilkan Daftar Duplikat\n"
                      << "5. Update / Delete Data\n"
                      << "6. Lihat Statistik\n"
                      << "7. Clear Database\n"
                      << "8. Keluar\n"
                      << "Pilih (1-8): ";
            std::cin >> menu;
            switch (menu) {
                case 1: insertManual();       break;
                case 2: insertBatch();        break;
                case 3: searchData();         break;
                case 4: listDuplikat();       break;
                case 5: updateDeleteData();   break;
                case 6: tampilkanStatistik(); break;
                case 7: clearDatabase();      break;
                case 8: std::cout << "Program ditutup.\n"; break;
                default: std::cout << ">> Opsi tidak valid.\n";
            }
        } while (menu != 8);
    }

private:
    bool muatDariDataset(const std::string& sumberFile) {
        auto start = std::chrono::steady_clock::now();
        int skipped = engine->bacaCSV(sumberFile);
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

        if (skipped == -1) {
            return false;
        }

        size_t slash = sumberFile.rfind('/');
        std::string datasetAsal = (slash != std::string::npos) ? sumberFile.substr(slash + 1) : sumberFile;
        engine->setDatasetAsal(datasetAsal);

        int total, unik, grupDup, recordDup;
        double waktu;
        engine->dapatkanStatistik(total, unik, grupDup, recordDup, waktu);

        std::cout << ">> Berhasil memuat " << total << " data dari \""
                  << datasetAsal << "\" dalam " << dur.count() << " ms";
        if (skipped) std::cout << " (" << skipped << " baris dilewati)";
        std::cout << ".\n";

        engine->simpanKeFile(fileDatabase);
        std::cout << ">> Database utama diperbarui: " << fileDatabase << "\n";
        return true;
    }

    void insertManual() {
        DataArsip d;
        d.id_dokumen = engine->generateNextID();
        std::cout << "\n[Insert Manual] ID Otomatis: " << d.id_dokumen << "\n";

        do {
            std::cout << "Nama File       : ";
            std::cin >> d.nama_file;
            if (!Validator::validasiNamaFile(d.nama_file)) {
                bool adaKarIlegal = false;
                for (char c : d.nama_file)
                    if (c == ',' || c == '"' || c == '\\' || c == '/')
                        { adaKarIlegal = true; break; }
                if (adaKarIlegal)
                    std::cout << "   [!] Nama file mengandung karakter tidak valid ( , \" \\ / ).\n";
                else
                    Validator::tampilkanEkstensiValid();
            }
        } while (!Validator::validasiNamaFile(d.nama_file));

        while (true) {
            std::cout << "Ukuran (bytes)  : ";
            if (std::cin >> d.ukuran_data && d.ukuran_data > 0) break;
            std::cout << "   [!] Ukuran harus angka positif lebih dari 0.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        do {
            std::cout << "Sumber Data     : ";
            std::cin >> d.sumber_data;
            if (!Validator::validasiSumber(d.sumber_data))
                std::cout << "   [!] Sumber data tidak boleh kosong.\n";
        } while (!Validator::validasiSumber(d.sumber_data));

        do {
            std::cout << "Tanggal Unggah  : ";
            std::cin >> d.tanggal_unggah;
            if (!Validator::validasiTanggal(d.tanggal_unggah))
                std::cout << "   [!] Format tanggal tidak valid. Gunakan YYYY-MM-DD (contoh: 2024-03-15).\n";
        } while (!Validator::validasiTanggal(d.tanggal_unggah));

        std::cin.ignore();
        std::cout << "Keyword Tambahan: ";
        std::cout << "(opsional, pisahkan spasi, Enter untuk lewati)\n";
        std::cout << "                : ";
        std::string keywords;
        std::getline(std::cin, keywords);
        d.konten = Validator::generateKonten(d.nama_file, d.ukuran_data, keywords);
        std::cout << "   >> Konten    : " << d.konten << "\n";

        auto start = std::chrono::steady_clock::now();
        std::string IDDupMeta = engine->cariDuplikatMetadata(d.nama_file, d.ukuran_data);
        std::string IDDupKonten = engine->cariDuplikatKonten(d.konten);
        auto stop = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        if (!IDDupMeta.empty()) {
            std::cout << ">> DITOLAK: Duplikat terdeteksi dengan ID \""
                      << IDDupMeta << "\" (nama + ukuran identik).\n";
        } else if (!IDDupKonten.empty()) {
            std::cout << ">> DITOLAK: Duplikat terdeteksi dengan ID \""
                      << IDDupKonten << "\" (isi konten identik).\n";
        } else {
            engine->insertRecord(d);
            engine->simpanKeFile(fileDatabase);
            std::cout << ">> DITERIMA: Data berhasil ditambahkan dengan ID " << d.id_dokumen << ".\n";
        }
        std::cout << ">> Waktu cek duplikat (" << namaStruktur << "): " << dur.count() << " mikrodetik.\n";
    }

    void insertBatch() {
        std::string fileBaru = pilihFile("Batch Import");

        std::ifstream file(fileBaru);
        if (!file.is_open()) {
            std::cout << ">> Gagal membuka \"" << fileBaru << "\"!\n";
            return;
        }

        std::string fileRejected = "rejected_" + fileBaru;
        std::ofstream rejFile(fileRejected);
        rejFile << "baris_ke,nama_file,ukuran_data,tanggal_unggah,sumber_data,konten,alasan_tolak\n";
        bool adaRejected = false;

        auto start = std::chrono::steady_clock::now();
        std::string baris;
        std::getline(file, baris); 

        int noBaris  = 1;
        int sukses   = 0;
        int duplikat = 0;
        int ditolak  = 0;

        while (std::getline(file, baris)) {
            if (baris.empty()) continue;
            noBaris++;

            std::vector<std::string> f = Validator::parseCSVLine(baris);

            if (f.size() < 6) {
                rejFile << noBaris << ",,,,,," << "format_rusak_field_kurang\n";
                ditolak++;
                adaRejected = true;
                continue;
            }

            std::string nama   = f[1];
            std::string ukuStr = f[2];
            std::string tgl    = f[3];
            std::string sumber = f[4];
            std::string konten = f[5];
            long long ukuran = 0;
            bool ukuranValid = true;

            try { ukuran = std::stoll(ukuStr); } catch (...) { ukuranValid = false; }

            std::string alasan = "";
            if (!Validator::validasiNamaFile(nama)) {
                bool adaKarIlegal = false;
                for (char c : nama)
                    if (c == ',' || c == '"' || c == '\\' || c == '/')
                        { adaKarIlegal = true; break; }
                alasan += adaKarIlegal ? "nama_file_karakter_ilegal;" : "nama_file_ekstensi_tidak_valid;";
            }
            if (!ukuranValid || ukuran <= 0)
                alasan += "ukuran_tidak_valid;";
            if (!Validator::validasiTanggal(tgl))
                alasan += "tanggal_tidak_valid;";
            if (!Validator::validasiSumber(sumber))
                alasan += "sumber_kosong;";

            if (!alasan.empty()) {
                rejFile << noBaris << ","
                        << nama    << ","
                        << ukuStr  << ","
                        << tgl     << ","
                        << sumber  << ","
                        << konten  << ","
                        << alasan  << "\n";
                ditolak++;
                adaRejected = true;
                continue;
            }

            DataArsip d;
            d.id_dokumen     = engine->generateNextID(); 
            d.nama_file      = nama;
            d.ukuran_data    = ukuran;
            d.tanggal_unggah = tgl;
            d.sumber_data    = sumber;
            d.konten         = konten;

            // Cek duplikat metadata ATAU konten
            bool isDup = (!engine->cariDuplikatMetadata(d.nama_file, d.ukuran_data).empty()) || 
                         (!engine->cariDuplikatKonten(d.konten).empty());
            
            engine->insertRecord(d);
            if (isDup) {
                duplikat++;
            } else {
                sukses++;
            }
        }

        file.close();
        rejFile.close();
        if (!adaRejected) std::remove(fileRejected.c_str()); 

        engine->simpanKeFile(fileDatabase);

        auto stop = std::chrono::steady_clock::now();
        auto dur  = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout << ">> Batch Import selesai dalam " << dur.count() << " ms.\n";
        std::cout << ">> " << sukses   << " record unik ditambahkan.\n";
        std::cout << ">> " << duplikat << " record duplikat ditambahkan (terdeteksi & dicatat).\n";
        std::cout << ">> " << ditolak  << " record ditolak karena format tidak valid";
        if (adaRejected)
            std::cout << " → lihat \"" << fileRejected << "\"";
        std::cout << ".\n";
    }

    void searchData() {
        std::string query;
        std::cout << "\n[Search] Masukkan ID atau Nama File: "; std::cin >> query;

        auto start = std::chrono::steady_clock::now();
        std::vector<DataArsip> results;
        bool found = engine->searchRecord(query, results);
        auto stop = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        if (found) {
            for (const auto& d : results) {
                std::cout << "  ID     : " << d.id_dokumen     << "\n"
                          << "  File   : " << d.nama_file      << "\n"
                          << "  Ukuran : " << d.ukuran_data    << " bytes\n"
                          << "  Tanggal: " << d.tanggal_unggah << "\n"
                          << "  Sumber : " << d.sumber_data    << "\n"
                          << "  Konten : " << d.konten         << "\n"
                          << "  -------\n";
            }
            std::cout << ">> " << results.size() << " record ditemukan.\n";
        } else {
            std::cout << ">> Data tidak ditemukan.\n";
        }
        std::cout << ">> Waktu pencarian: " << dur.count() << " mikrodetik.\n";
    }

    void listDuplikat() {
        std::cout << "\nPILIH MODE PEMINDAIAN DUPLIKAT:\n"
                  << "1. Berdasarkan Metadata (Nama File + Ukuran)\n"
                  << "2. Berdasarkan Isi Konten\n"
                  << "Pilih (1-2): ";
        int mode; std::cin >> mode;
        if (mode != 1 && mode != 2) {
            std::cout << ">> Pilihan tidak valid.\n";
            return;
        }

        engine->scanDuplicates(mode);
    }

    void updateDeleteData() {
        std::string id;
        std::cout << "\n[Update/Delete] Masukkan ID Dokumen: "; std::cin >> id;

        auto start = std::chrono::steady_clock::now();
        DataArsip d;
        bool found = engine->getRecordByID(id, d);

        if (found) {
            std::cout << ">> Ditemukan:\n"
                      << "   Nama File : " << d.nama_file      << "\n"
                      << "   Ukuran    : " << d.ukuran_data    << " bytes\n"
                      << "   Tanggal   : " << d.tanggal_unggah << "\n"
                      << "   Sumber    : " << d.sumber_data    << "\n"
                      << "   Konten    : " << d.konten         << "\n";
            std::cout << "\n1. Update Metadata Utama (Nama File & Ukuran)\n"
                      << "2. Delete Data\n"
                      << "Pilih (1-2): ";
            int opsi; std::cin >> opsi;

            if (opsi == 1) {
                std::string namaBaru;
                do {
                    std::cout << "Nama file baru: "; std::cin >> namaBaru;
                    if (!Validator::validasiNamaFile(namaBaru)) {
                        bool adaKarIlegal = false;
                        for (char c : namaBaru)
                            if (c == ',' || c == '"' || c == '\\' || c == '/')
                                { adaKarIlegal = true; break; }
                        std::cout << "   [!] Nama file mengandung karakter tidak valid.\n";
                    }
                } while (!Validator::validasiNamaFile(namaBaru));

                long long ukuranBaru;
                while (true) {
                    std::cout << "Ukuran baru (bytes): ";
                    if (std::cin >> ukuranBaru && ukuranBaru > 0) break;
                    std::cout << "   [!] Ukuran harus angka positif lebih dari 0.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }

                // Cek konflik duplikat
                std::string IDDup = engine->cariDuplikatMetadata(namaBaru, ukuranBaru);
                if (!IDDup.empty() && IDDup != id) {
                    std::cout << ">> DITOLAK: Nama baru + ukuran yang sama sudah ada (duplikat metadata).\n";
                } else {
                    std::cin.ignore();
                    std::cout << "Keyword baru (opsional, Enter untuk lewati): ";
                    std::string kw; std::getline(std::cin, kw);
                    std::string kontenBaru = Validator::generateKonten(namaBaru, ukuranBaru, kw);
                    
                    engine->updateMetadata(id, namaBaru, ukuranBaru, kontenBaru);
                    engine->simpanKeFile(fileDatabase);
                    std::cout << ">> Metadata & konten berhasil diupdate.\n";
                }

            } else if (opsi == 2) {
                std::string konfirmasi;
                std::cout << ">> Yakin hapus \"" << d.nama_file << "\"? (y/n): ";
                std::cin >> konfirmasi;
                if (konfirmasi == "y" || konfirmasi == "Y") {
                    engine->deleteRecord(id);
                    engine->simpanKeFile(fileDatabase);
                    std::cout << ">> Data berhasil dihapus.\n";
                } else {
                    std::cout << ">> Penghapusan dibatalkan.\n";
                }
            } else {
                std::cout << ">> Opsi tidak valid.\n";
            }

            auto stop = std::chrono::steady_clock::now();
            std::cout << ">> Waktu operasi: "
                      << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
                      << " mikrodetik.\n";
            return;
        }
        std::cout << ">> ID tidak ditemukan.\n";
    }

    void tampilkanStatistik() {
        std::cout << "\n========== STATISTIK DATABASE ==========\n";
        int total, unik, grupDup, recordDup;
        double waktuMs;
        engine->dapatkanStatistik(total, unik, grupDup, recordDup, waktuMs);

        std::cout << "Dataset                 : " << engine->getDatasetAsal() << "\n";
        std::cout << "Total Record di Sistem  : " << total                 << "\n";
        std::cout << "Record Benar-benar Unik : " << unik                  << "\n";
        std::cout << "Grup Duplikat           : " << grupDup               << "\n";
        std::cout << "Record Terlibat Duplikat: " << recordDup             << "\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Waktu Kalkulasi         : " << waktuMs               << " ms\n";
        std::cout << "=========================================\n";

        {
            std::ifstream cek(statsLogFile);
            if (!cek.is_open()) {
                std::ofstream buat(statsLogFile);
                buat << "timestamp,struktur_data,dataset,total_record,"
                     << "record_unik,grup_duplikat,record_duplikat,waktu_ms\n";
            }
        }

        auto now = std::chrono::system_clock::now();
        std::time_t nowT = std::chrono::system_clock::to_time_t(now);
        std::tm* tmInfo = std::localtime(&nowT);
        char tsBuf[20];
        std::strftime(tsBuf, sizeof(tsBuf), "%Y-%m-%d %H:%M:%S", tmInfo);

        std::ofstream log(statsLogFile, std::ios::app);
        log << std::fixed << std::setprecision(4);
        log << tsBuf              << ","
            << namaStruktur       << ","
            << engine->getDatasetAsal() << ","
            << total              << ","
            << unik               << ","
            << grupDup            << ","
            << recordDup          << ","
            << waktuMs            << "\n";
        log.close();

        std::cout << ">> Hasil dicatat ke \"" << statsLogFile << "\".\n";
    }

    std::string pilihFile(const std::string& prefix) {
        const std::vector<std::pair<std::string,std::string>> DAFTAR_FILE = {
            {"datasets/arsip_0001000.csv",        "    1.000 record"},
            {"datasets/arsip_0005000.csv",        "    5.000 record"},
            {"datasets/arsip_0010000.csv",        "   10.000 record"},
            {"datasets/arsip_0050000.csv",        "   50.000 record"},
            {"datasets/arsip_0100000.csv",        "  100.000 record"},
            {"datasets/batch_import_0001000.csv", "    1.000 record (batch)"},
            {"datasets/batch_import_0005000.csv", "    5.000 record (batch)"},
        };

        std::cout << "\n[" << prefix << "] Pilih file:\n";
        for (size_t i = 0; i < DAFTAR_FILE.size(); i++)
            std::cout << "  " << (i+1) << ". " << DAFTAR_FILE[i].first
                      << "  (" << DAFTAR_FILE[i].second << ")\n";
        std::cout << "  " << (DAFTAR_FILE.size()+1) << ". Input nama file manual\n";
        std::cout << "Pilih (1-" << (DAFTAR_FILE.size()+1) << "): ";

        int pilihan;
        while (true) {
            std::cin >> pilihan;
            if (pilihan >= 1 && pilihan <= (int)DAFTAR_FILE.size())
                return DAFTAR_FILE[pilihan-1].first;
            if (pilihan == (int)DAFTAR_FILE.size() + 1) {
                std::string namaFile;
                std::cout << "Nama file (beserta path): "; std::cin >> namaFile;
                return namaFile;
            }
            std::cout << "   [!] Pilihan tidak valid, coba lagi: ";
        }
    }

    void clearDatabase() {
        std::cout << "\n[Clear Database] Yakin hapus seluruh database? (y/n): ";
        std::string konfirmasi; std::cin >> konfirmasi;
        if (konfirmasi != "y" && konfirmasi != "Y") {
            std::cout << ">> Dibatalkan.\n";
            return;
        }
        engine->clearAll();
        std::remove(fileDatabase.c_str());
        std::cout << ">> Database berhasil dikosongkan.\n";
        std::cout << ">> Import dataset baru untuk melanjutkan.\n";

        while (true) {
            std::string sumber = pilihFile("Import Dataset Baru");
            if (muatDariDataset(sumber)) break;
            std::cout << "   [!] File tidak ditemukan atau kosong, silakan pilih ulang.\n";
        }
    }
};

#endif // ARSIP_MANAGER_H
