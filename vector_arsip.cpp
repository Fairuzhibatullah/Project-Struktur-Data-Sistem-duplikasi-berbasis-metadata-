#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <limits>

using namespace std;
using namespace std::chrono;

// DATA 
struct DataArsip {
    string    id_dokumen;
    string    nama_file;
    long long ukuran_data = 0;
    string    tanggal_unggah;
    string    sumber_data;
    string    konten;
};

// LOGIKA VALIDASI & HELPER (pembantuan untuk validasi input dan parsing CSV)
class Validator {
public:
    static vector<string> parseCSVLine(const string& baris) {
        vector<string> fields;
        string field;
        bool inQuotes = false;
        for (char c : baris) {
            if (c == '"')                inQuotes = !inQuotes;
            else if (c == ',' && !inQuotes) { fields.push_back(field); field.clear(); }
            else                            field += c;
        }
        fields.push_back(field);
        return fields;
    }

    static bool validasiNamaFile(const string& nama) {
        if (nama.empty()) return false;
        for (char c : nama)
            if (c == ',' || c == '"' || c == '\\' || c == '/') return false;

        size_t titik = nama.rfind('.');
        if (titik == string::npos || titik == nama.size() - 1) return false;

        string ext = nama.substr(titik + 1);
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        const vector<string> EKSTENSI_VALID = {
            "pdf", "docx", "doc", "xlsx", "xls",
            "txt", "csv", "json", "xml",
            "png", "jpg", "jpeg", "zip", "rar"
        };

        for (const auto& e : EKSTENSI_VALID)
            if (ext == e) return true;
        return false;
    }

    static void tampilkanEkstensiValid() {
        const vector<string> EKSTENSI_VALID = {
            "pdf", "docx", "doc", "xlsx", "xls",
            "txt", "csv", "json", "xml",
            "png", "jpg", "jpeg", "zip", "rar"
        };
        cout << "   [!] Ekstensi tidak diizinkan. Ekstensi yang valid:\n       ";
        for (size_t i = 0; i < EKSTENSI_VALID.size(); i++) {
            cout << "." << EKSTENSI_VALID[i];
            if (i < EKSTENSI_VALID.size() - 1) cout << "  ";
        }
        cout << "\n";
    }

    static bool validasiTanggal(const string& tgl) {
        if (tgl.size() != 10) return false;
        if (tgl[4] != '-' || tgl[7] != '-') return false;
        for (int i = 0; i < 10; i++) {
            if (i == 4 || i == 7) continue;
            if (!isdigit(tgl[i])) return false;
        }
        return true;
    }

    static bool validasiSumber(const string& sumber) {
        return !sumber.empty();
    }

    static string generateKonten(const string& namaFile, long long ukuran,
                                 const string& keywords) {
        size_t titik = namaFile.rfind('.');
        string namaBersih = (titik != string::npos) ? namaFile.substr(0, titik) : namaFile;

        string konten = namaBersih + "|" + to_string(ukuran);
        if (!keywords.empty()) konten += "|" + keywords;
        return konten;
    }
};

// ENGINE STRUKTUR DATA (VECTOR)
class VectorEngine {
private:
    vector<DataArsip> database;
    int maxIDNum = 0;
    string datasetAsal = "";

public:
    VectorEngine() {}

    int bacaCSV(const string& path) {
        database.clear();
        maxIDNum = 0;
        ifstream file(path);
        if (!file.is_open()) return -1;
        string baris;
        getline(file, baris); 
        int skipped = 0;
        while (getline(file, baris)) {
            if (baris.empty()) continue;
            vector<string> f = Validator::parseCSVLine(baris);
            if (f.size() < 6) { skipped++; continue; }
            DataArsip d;
            d.id_dokumen     = f[0];
            d.nama_file      = f[1];
            try { d.ukuran_data = stoll(f[2]); } catch (...) { d.ukuran_data = 0; }
            d.tanggal_unggah = f[3];
            d.sumber_data    = f[4];
            d.konten         = f[5];
            
            database.push_back(d);
            updateMaxIDNum(d.id_dokumen);
        }
        file.close();
        return skipped;
    }

    void simpanKeFile(const string& path) {
        ofstream file(path);
        if (!file.is_open()) {
            cout << ">> ERROR: Gagal membuka file untuk disimpan.\n";
            return;
        }
        file << "id_dokumen,nama_file,ukuran_data,tanggal_unggah,sumber_data,konten\n";
        for (const auto& d : database) {
            string kontenSafe = d.konten;
            if (kontenSafe.find(',') != string::npos)
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

    string getDatasetAsal() const { return datasetAsal; }
    void setDatasetAsal(const string& asal) { datasetAsal = asal; }

    string generateNextID() {
        maxIDNum++;
        ostringstream oss;
        oss << "DOC-" << setw(7) << setfill('0') << maxIDNum;
        return oss.str();
    }

    void updateMaxIDNum(const string& id) {
        size_t dash = id.rfind('-');
        if (dash != string::npos) {
            try {
                int num = stoi(id.substr(dash + 1));
                if (num > maxIDNum) maxIDNum = num;
            } catch (...) {}
        }
    }

    void resetMaxIDNum() { maxIDNum = 0; }

    string cariDuplikatMetadata(const string& nama, long long ukuran) {
        for (const auto& d : database) {
            if (d.nama_file == nama && d.ukuran_data == ukuran) {
                return d.id_dokumen;
            }
        }
        return "";
    }

    string cariDuplikatKonten(const string& konten) {
        for (const auto& d : database) {
            if (d.konten == konten) {
                return d.id_dokumen;
            }
        }
        return "";
    }

    void insertRecord(const DataArsip& d) {
        database.push_back(d);
        updateMaxIDNum(d.id_dokumen);
    }

    bool searchRecord(const string& query, vector<DataArsip>& results) {
        results.clear();
        for (const auto& d : database) {
            if (d.id_dokumen == query || d.nama_file == query) {
                results.push_back(d);
            }
        }
        return !results.empty();
    }

    bool getRecordByID(const string& id, DataArsip& d) {
        for (const auto& record : database) {
            if (record.id_dokumen == id) {
                d = record;
                return true;
            }
        }
        return false;
    }

    bool updateMetadata(const string& id, const string& namaBaru, long long ukuranBaru, const string& kontenBaru) {
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

    bool deleteRecord(const string& id) {
        for (auto it = database.begin(); it != database.end(); ++it) {
            if (it->id_dokumen == id) {
                database.erase(it);
                return true;
            }
        }
        return false;
    }

    void clearAll() {
        database.clear();
        maxIDNum = 0;
        datasetAsal = "";
    }

    void scanDuplicates(int mode) {
        if (mode != 1 && mode != 2) return;
        
        auto start = steady_clock::now();
        int total = (int)database.size();
        vector<bool> sudahDicetak(total, false);
        int grupDuplikat        = 0;
        int totalRecordDuplikat = 0;

        for (int i = 0; i < total; i++) {
            if (sudahDicetak[i]) continue;
            vector<int> grup = {i};
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
                    cout << "\n[Grup " << grupDuplikat << "] "
                         << database[i].nama_file << " | "
                         << database[i].ukuran_data << " bytes"
                         << " (" << grup.size() << " salinan)\n";
                } else {
                    cout << "\n[Grup " << grupDuplikat << "] Konten: "
                         << database[i].konten
                         << " (" << grup.size() << " salinan)\n";
                }
                
                for (int idx : grup) {
                    cout << "  -> ID: " << database[idx].id_dokumen
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

        auto stop = steady_clock::now();
        auto dur  = duration_cast<milliseconds>(stop - start);

        if (grupDuplikat == 0)
            cout << ">> Tidak ada duplikat ditemukan.\n";
        else
            cout << "\n>> " << grupDuplikat << " grup duplikat ("
                 << totalRecordDuplikat << " record terlibat).\n";
        cout << ">> Waktu pemindaian: " << dur.count() << " ms.\n";
    }

    void dapatkanStatistik(int& total, int& unik, int& grupDup, int& recordDup, double& waktuMs) {
        auto start = steady_clock::now();

        total = (int)database.size();
        vector<bool> sudahDihitung(total, false);
        grupDup        = 0;
        recordDup = 0;

        for (int i = 0; i < total; i++) {
            if (sudahDihitung[i]) continue;
            vector<int> grup = {i};
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

        auto stop = steady_clock::now();
        duration<double, std::milli> dur = stop - start;
        waktuMs = dur.count();
    }
};

// FUNGSI UTILITY & MENU
string pilihFile(const string& prefix) {
    const vector<pair<string,string>> DAFTAR_FILE = {
        {"datasets/arsip_0001000.csv",        "    1.000 record"},
        {"datasets/arsip_0005000.csv",        "    5.000 record"},
        {"datasets/arsip_0010000.csv",        "   10.000 record"},
        {"datasets/arsip_0050000.csv",        "   50.000 record"},
        {"datasets/arsip_0100000.csv",        "  100.000 record"},
        {"datasets/batch_import_0001000.csv", "    1.000 record (batch)"},
        {"datasets/batch_import_0005000.csv", "    5.000 record (batch)"},
    };

    cout << "\n[" << prefix << "] Pilih file:\n";
    for (size_t i = 0; i < DAFTAR_FILE.size(); i++)
        cout << "  " << (i+1) << ". " << DAFTAR_FILE[i].first
             << "  (" << DAFTAR_FILE[i].second << ")\n";
    cout << "  " << (DAFTAR_FILE.size()+1) << ". Input nama file manual\n";
    cout << "Pilih (1-" << (DAFTAR_FILE.size()+1) << "): ";

    int pilihan;
    while (true) {
        cin >> pilihan;
        if (pilihan >= 1 && pilihan <= (int)DAFTAR_FILE.size())
            return DAFTAR_FILE[pilihan-1].first;
        if (pilihan == (int)DAFTAR_FILE.size() + 1) {
            string namaFile;
            cout << "Nama file (beserta path): "; cin >> namaFile;
            return namaFile;
        }
        cout << "   [!] Pilihan tidak valid, coba lagi: ";
    }
}

bool muatDariDataset(VectorEngine& engine, const string& sumberFile, const string& fileDatabase) {
    auto start = steady_clock::now();
    int skipped = engine.bacaCSV(sumberFile);
    auto dur = duration_cast<milliseconds>(steady_clock::now() - start);

    if (skipped == -1) {
        return false;
    }

    size_t slash = sumberFile.rfind('/');
    string datasetAsal = (slash != string::npos) ? sumberFile.substr(slash + 1) : sumberFile;
    engine.setDatasetAsal(datasetAsal);

    int total, unik, grupDup, recordDup;
    double waktu;
    engine.dapatkanStatistik(total, unik, grupDup, recordDup, waktu);

    cout << ">> Berhasil memuat " << total << " data dari \""
         << datasetAsal << "\" dalam " << dur.count() << " ms";
    if (skipped) cout << " (" << skipped << " baris dilewati)";
    cout << ".\n";

    engine.simpanKeFile(fileDatabase);
    cout << ">> Database utama diperbarui: " << fileDatabase << "\n";
    return true;
}

void insertManual(VectorEngine& engine, const string& fileDatabase, const string& namaStruktur) {
    DataArsip d;
    d.id_dokumen = engine.generateNextID();
    cout << "\n[Insert Manual] ID Otomatis: " << d.id_dokumen << "\n";

    do {
        cout << "Nama File       : ";
        cin >> d.nama_file;
        if (!Validator::validasiNamaFile(d.nama_file)) {
            bool adaKarIlegal = false;
            for (char c : d.nama_file)
                if (c == ',' || c == '"' || c == '\\' || c == '/')
                    { adaKarIlegal = true; break; }
            if (adaKarIlegal)
                cout << "   [!] Nama file mengandung karakter tidak valid ( , \" \\ / ).\n";
            else
                Validator::tampilkanEkstensiValid();
        }
    } while (!Validator::validasiNamaFile(d.nama_file));

    while (true) {
        cout << "Ukuran (bytes)  : ";
        if (cin >> d.ukuran_data && d.ukuran_data > 0) break;
        cout << "   [!] Ukuran harus angka positif lebih dari 0.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    do {
        cout << "Sumber Data     : ";
        cin >> d.sumber_data;
        if (!Validator::validasiSumber(d.sumber_data))
            cout << "   [!] Sumber data tidak boleh kosong.\n";
    } while (!Validator::validasiSumber(d.sumber_data));

    do {
        cout << "Tanggal Unggah  : ";
        cin >> d.tanggal_unggah;
        if (!Validator::validasiTanggal(d.tanggal_unggah))
            cout << "   [!] Format tanggal tidak valid. Gunakan YYYY-MM-DD (contoh: 2024-03-15).\n";
    } while (!Validator::validasiTanggal(d.tanggal_unggah));

    cin.ignore();
    cout << "Keyword Tambahan: ";
    cout << "(opsional, pisahkan spasi, Enter untuk lewati)\n";
    cout << "                : ";
    string keywords;
    getline(cin, keywords);
    d.konten = Validator::generateKonten(d.nama_file, d.ukuran_data, keywords);
    cout << "   >> Konten    : " << d.konten << "\n";

    auto start = steady_clock::now();
    string IDDupMeta = engine.cariDuplikatMetadata(d.nama_file, d.ukuran_data);
    // string IDDupKonten = engine.cariDuplikatKonten(d.konten);
    auto stop = steady_clock::now();
    auto dur = duration_cast<microseconds>(stop - start);

    if (!IDDupMeta.empty()) {
        cout << ">> DITOLAK: Duplikat terdeteksi dengan ID \""
             << IDDupMeta << "\" (nama + ukuran identik).\n";
    } 
    // else if (!IDDupKonten.empty()) {
    //     cout << ">> DITOLAK: Duplikat terdeteksi dengan ID \""
    //          << IDDupKonten << "\" (isi konten identik).\n";
    // } 
    else {
        engine.insertRecord(d);
        engine.simpanKeFile(fileDatabase);
        cout << ">> DITERIMA: Data berhasil ditambahkan dengan ID " << d.id_dokumen << ".\n";
    }
    cout << ">> Waktu cek duplikat (" << namaStruktur << "): " << dur.count() << " mikrodetik.\n";
}

void insertBatch(VectorEngine& engine, const string& fileDatabase) {
    string fileBaru = pilihFile("Batch Import");

    ifstream file(fileBaru);
    if (!file.is_open()) {
        cout << ">> Gagal membuka \"" << fileBaru << "\"!\n";
        return;
    }

    string fileRejected = "rejected_" + fileBaru;
    ofstream rejFile(fileRejected);
    rejFile << "baris_ke,nama_file,ukuran_data,tanggal_unggah,sumber_data,konten,alasan_tolak\n";
    bool adaRejected = false;

    auto start = steady_clock::now();
    string baris;
    getline(file, baris); 

    int noBaris  = 1;
    int sukses   = 0;
    int duplikat = 0;
    int ditolak  = 0;

    while (getline(file, baris)) {
        if (baris.empty()) continue;
        noBaris++;

        vector<string> f = Validator::parseCSVLine(baris);

        if (f.size() < 6) {
            rejFile << noBaris << ",,,,,," << "format_rusak_field_kurang\n";
            ditolak++;
            adaRejected = true;
            continue;
        }

        string nama   = f[1];
        string ukuStr = f[2];
        string tgl    = f[3];
        string sumber = f[4];
        string konten = f[5];
        long long ukuran = 0;
        bool ukuranValid = true;

        try { ukuran = stoll(ukuStr); } catch (...) { ukuranValid = false; }

        string alasan = "";
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
        d.id_dokumen     = engine.generateNextID(); 
        d.nama_file      = nama;
        d.ukuran_data    = ukuran;
        d.tanggal_unggah = tgl;
        d.sumber_data    = sumber;
        d.konten         = konten;

        // Cek duplikat metadata ATAU konten
        bool isDup = (!engine.cariDuplikatMetadata(d.nama_file, d.ukuran_data).empty()) || 
                     (!engine.cariDuplikatKonten(d.konten).empty());
        
        engine.insertRecord(d);
        if (isDup) {
            duplikat++;
        } else {
            sukses++;
        }
    }

    file.close();
    rejFile.close();
    if (!adaRejected) remove(fileRejected.c_str()); 

    engine.simpanKeFile(fileDatabase);

    auto stop = steady_clock::now();
    auto dur  = duration_cast<milliseconds>(stop - start);

    cout << ">> Batch Import selesai dalam " << dur.count() << " ms.\n";
    cout << ">> " << sukses   << " record unik ditambahkan.\n";
    cout << ">> " << duplikat << " record duplikat ditambahkan (terdeteksi & dicatat).\n";
    cout << ">> " << ditolak  << " record ditolak karena format tidak valid";
    if (adaRejected)
        cout << " → lihat \"" << fileRejected << "\"";
    cout << ".\n";
}

void searchData(VectorEngine& engine) {
    string query;
    cout << "\n[Search] Masukkan ID atau Nama File: "; cin >> query;

    auto start = steady_clock::now();
    vector<DataArsip> results;
    bool found = engine.searchRecord(query, results);
    auto stop = steady_clock::now();
    auto dur = duration_cast<microseconds>(stop - start);

    if (found) {
        for (const auto& d : results) {
            cout << "  ID     : " << d.id_dokumen     << "\n"
                 << "  File   : " << d.nama_file      << "\n"
                 << "  Ukuran : " << d.ukuran_data    << " bytes\n"
                 << "  Tanggal: " << d.tanggal_unggah << "\n"
                 << "  Sumber : " << d.sumber_data    << "\n"
                 << "  Konten : " << d.konten         << "\n"
                 << "  -------\n";
        }
        cout << ">> " << results.size() << " record ditemukan.\n";
    } else {
        cout << ">> Data tidak ditemukan.\n";
    }
    cout << ">> Waktu pencarian: " << dur.count() << " mikrodetik.\n";
}

void listDuplikat(VectorEngine& engine) {
    cout << "\nPILIH MODE PEMINDAIAN DUPLIKAT:\n"
         << "1. Berdasarkan Metadata (Nama File + Ukuran)\n"
         << "2. Berdasarkan Isi Konten\n"
         << "Pilih (1-2): ";
    int mode; cin >> mode;
    if (mode != 1 && mode != 2) {
        cout << ">> Pilihan tidak valid.\n";
        return;
    }

    engine.scanDuplicates(mode);
}

void updateDeleteData(VectorEngine& engine, const string& fileDatabase) {
    string id;
    cout << "\n[Update/Delete] Masukkan ID Dokumen: "; cin >> id;

    DataArsip d;
    bool found = engine.getRecordByID(id, d);

    if (found) {
        cout << ">> Ditemukan:\n"
             << "   Nama File : " << d.nama_file      << "\n"
             << "   Ukuran    : " << d.ukuran_data    << " bytes\n"
             << "   Tanggal   : " << d.tanggal_unggah << "\n"
             << "   Sumber    : " << d.sumber_data    << "\n"
             << "   Konten    : " << d.konten         << "\n";
        cout << "\n1. Update Metadata Utama (Nama File & Ukuran)\n"
             << "2. Delete Data\n"
             << "Pilih (1-2): ";
        int opsi; cin >> opsi;

        if (opsi == 1) {
            string namaBaru;
            do {
                cout << "Nama file baru: "; cin >> namaBaru;
                if (!Validator::validasiNamaFile(namaBaru)) {
                    bool adaKarIlegal = false;
                    for (char c : namaBaru)
                        if (c == ',' || c == '"' || c == '\\' || c == '/')
                            { adaKarIlegal = true; break; }
                    cout << "   [!] Nama file mengandung karakter tidak valid.\n";
                }
            } while (!Validator::validasiNamaFile(namaBaru));

            long long ukuranBaru;
            while (true) {
                cout << "Ukuran baru (bytes): ";
                if (cin >> ukuranBaru && ukuranBaru > 0) break;
                cout << "   [!] Ukuran harus angka positif lebih dari 0.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }

            // Cek konflik duplikat
            string IDDup = engine.cariDuplikatMetadata(namaBaru, ukuranBaru);
            if (!IDDup.empty() && IDDup != id) {
                cout << ">> DITOLAK: Nama baru + ukuran yang sama sudah ada (duplikat metadata).\n";
            } else {
                cin.ignore();
                cout << "Keyword baru (opsional, Enter untuk lewati): ";
                string kw; getline(cin, kw);
                string kontenBaru = Validator::generateKonten(namaBaru, ukuranBaru, kw);
                
                auto start = steady_clock::now();
                engine.updateMetadata(id, namaBaru, ukuranBaru, kontenBaru);
                auto stop = steady_clock::now();
                engine.simpanKeFile(fileDatabase);
                cout << ">> Metadata & konten berhasil diupdate.\n";
                cout << ">> Waktu operasi: "
                     << duration_cast<microseconds>(stop - start).count()
                     << " mikrodetik.\n";
            }
            

        } else if (opsi == 2) {
            string konfirmasi;
            cout << ">> Yakin hapus \"" << d.nama_file << "\"? (y/n): ";
            cin >> konfirmasi;
            if (konfirmasi == "y" || konfirmasi == "Y") {
                auto start = steady_clock::now();
                engine.deleteRecord(id);
                auto stop = steady_clock::now();
                engine.simpanKeFile(fileDatabase);
                cout << ">> Data berhasil dihapus.\n";
                cout << ">> Waktu operasi: "
                     << duration_cast<microseconds>(stop - start).count()
                     << " mikrodetik.\n";
            } else {
                cout << ">> Penghapusan dibatalkan.\n";
            }
        } else {
            cout << ">> Opsi tidak valid.\n";
        }

        return;
    }
    cout << ">> ID tidak ditemukan.\n";
}

void tampilkanStatistik(VectorEngine& engine, const string& statsLogFile, const string& namaStruktur) {
    cout << "\n========== STATISTIK DATABASE ==========\n";
    int total, unik, grupDup, recordDup;
    double waktuMs;
    engine.dapatkanStatistik(total, unik, grupDup, recordDup, waktuMs);

    cout << "Dataset                 : " << engine.getDatasetAsal() << "\n";
    cout << "Total Record di Sistem  : " << total                 << "\n";
    cout << "Record Benar-benar Unik : " << unik                  << "\n";
    cout << "Grup Duplikat           : " << grupDup               << "\n";
    cout << "Record Terlibat Duplikat: " << recordDup             << "\n";
    cout << fixed << setprecision(4);
    cout << "Waktu Kalkulasi         : " << waktuMs               << " ms\n";
    cout << "=========================================\n";

    {
        ifstream cek(statsLogFile);
        if (!cek.is_open()) {
            ofstream buat(statsLogFile);
            buat << "timestamp,struktur_data,dataset,total_record,"
                 << "record_unik,grup_duplikat,record_duplikat,waktu_ms\n";
        }
    }

    auto now = system_clock::now();
    time_t nowT = system_clock::to_time_t(now);
    tm* tmInfo = localtime(&nowT);
    char tsBuf[20];
    strftime(tsBuf, sizeof(tsBuf), "%Y-%m-%d %H:%M:%S", tmInfo);

    ofstream log(statsLogFile, ios::app);
    log << fixed << setprecision(4);
    log << tsBuf              << ","
        << namaStruktur       << ","
        << engine.getDatasetAsal() << ","
        << total              << ","
        << unik               << ","
        << grupDup            << ","
        << recordDup          << ","
        << waktuMs            << "\n";
    log.close();

    cout << ">> Hasil dicatat ke \"" << statsLogFile << "\".\n";
}

void clearDatabase(VectorEngine& engine, const string& fileDatabase) {
    cout << "\n[Clear Database] Yakin hapus seluruh database? (y/n): ";
    string konfirmasi; cin >> konfirmasi;
    if (konfirmasi != "y" && konfirmasi != "Y") {
        cout << ">> Dibatalkan.\n";
        return;
    }
    engine.clearAll();
    remove(fileDatabase.c_str());
    cout << ">> Database berhasil dikosongkan.\n";
    cout << ">> Import dataset baru untuk melanjutkan.\n";

    while (true) {
        string sumber = pilihFile("Import Dataset Baru");
        if (muatDariDataset(engine, sumber, fileDatabase)) break;
        cout << "   [!] File tidak ditemukan atau kosong, silakan pilih ulang.\n";
    }
}

int main() {
    VectorEngine engine;
    const string fileDatabase = "vector_database.csv";
    const string statsLogFile = "statistik_log.csv";
    const string namaStruktur = "Vector";

    cout << "============================================\n";
    cout << "  SISTEM DETEKSI DUPLIKASI ARSIP ( " << namaStruktur << " )  \n";
    cout << "  Deteksi berbasis: nama_file + ukuran_data \n";
    cout << "============================================\n";

    ifstream cekDB(fileDatabase);
    bool dbAda = cekDB.is_open();
    cekDB.close();

    if (dbAda) {
        cout << "\nDatabase ditemukan: " << fileDatabase << "\n";
        cout << "1. Lanjut dari database yang ada\n";
        cout << "2. Import dataset baru (database lama akan diganti)\n";
        cout << "Pilih (1-2): ";
        int pilihan; cin >> pilihan;

        if (pilihan == 1) {
            auto start = steady_clock::now();
            int skipped = engine.bacaCSV(fileDatabase);
            auto dur = duration_cast<milliseconds>(steady_clock::now() - start);
            
            if (skipped == -1) {
                cout << "   [!] Database kosong, import dataset baru.\n";
                dbAda = false;
            } else {
                engine.setDatasetAsal(fileDatabase);
                
                int total, unik, grupDup, recordDup;
                double waktu;
                engine.dapatkanStatistik(total, unik, grupDup, recordDup, waktu);
                cout << ">> Berhasil memuat " << total << " data dalam " << dur.count() << " ms.\n";
            }
        } else {
            dbAda = false;
        }
    }

    if (!dbAda) {
        while (true) {
            string sumber = pilihFile("Import Dataset");
            if (muatDariDataset(engine, sumber, fileDatabase)) break;
            cout << "   [!] File tidak ditemukan atau kosong, silakan pilih ulang.\n";
        }
    }

    int menu;
    do {
        cout << "\n--- MENU UTAMA ---\n"
             << "1. Insert Manual\n"
             << "2. Batch Import (CSV)\n"
             << "3. Cari Data (Search)\n"
             << "4. Tampilkan Daftar Duplikat\n"
             << "5. Update / Delete Data\n"
             << "6. Lihat Statistik\n"
             << "7. Clear Database\n"
             << "8. Keluar\n"
             << "Pilih (1-8): ";
        cin >> menu;
        switch (menu) {
            case 1: insertManual(engine, fileDatabase, namaStruktur); break;
            case 2: insertBatch(engine, fileDatabase);                break;
            case 3: searchData(engine);                               break;
            case 4: listDuplikat(engine);                             break;
            case 5: updateDeleteData(engine, fileDatabase);           break;
            case 6: tampilkanStatistik(engine, statsLogFile, namaStruktur); break;
            case 7: clearDatabase(engine, fileDatabase);              break;
            case 8: cout << "Program ditutup.\n"; break;
            default: cout << ">> Opsi tidak valid.\n";
        }
    } while (menu != 8);

    return 0;
}