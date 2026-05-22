#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
using namespace std;
using namespace std::chrono;


struct DataArsip {
    string    id_dokumen;
    string    nama_file;
    long long ukuran_data;
    string    tanggal_unggah;
    string    sumber_data;
    string    konten;
};

vector<DataArsip> database;
const string FILE_NAME = "datasets/arsip_0001000.csv";   


vector<string> parseCSVLine(const string& baris) {
    vector<string> fields;
    string field;
    bool inQuotes = false;
    for (size_t i = 0; i < baris.size(); i++) {
        char c = baris[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field); 
    return fields;
}

// ============================================================
//  UTILITAS: generate ID berikutnya (format DOC-XXXXXXX)
//  Mencari ID numerik tertinggi, bukan asumsi sorted
// ============================================================
string generateNextID() {
    int maxNum = 0;
    for (const auto& d : database) {
        size_t dash = d.id_dokumen.rfind('-');
        if (dash != string::npos) {
            try {
                int num = stoi(d.id_dokumen.substr(dash + 1));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    ostringstream oss;
    oss << "MAN-" << setw(7) << setfill('0') << (maxNum + 1);
    return oss.str();
}

// ============================================================
//  UTILITAS: cek duplikat berbasis metadata (nama_file + ukuran_data)
//  Kembalikan indeks record pertama yang cocok, atau -1 jika tidak ada
// ============================================================
int cariDuplikatMetadata(const string& nama, long long ukuran) {
    for (size_t i = 0; i < database.size(); i++) {
        if (database[i].nama_file == nama && database[i].ukuran_data == ukuran)
            return (int)i;
    }
    return -1;
}

// ============================================================
//  SIMPAN KE FILE
// ============================================================
void simpanKeFile() {
    ofstream file(FILE_NAME);
    if (!file.is_open()) {
        cout << ">> ERROR: Gagal membuka file untuk disimpan.\n";
        return;
    }
    file << "id_dokumen,nama_file,ukuran_data,tanggal_unggah,sumber_data,konten\n";
    for (const auto& d : database) {
        string kontenSafe = d.konten;
        if (kontenSafe.find(',') != string::npos)
            kontenSafe = "\"" + kontenSafe + "\"";
        file << d.id_dokumen    << ","
             << d.nama_file     << ","
             << d.ukuran_data   << ","
             << d.tanggal_unggah << ","
             << d.sumber_data   << ","
             << kontenSafe      << "\n";
    }
    file.close();
}

// ============================================================
//  MUAT DATA DARI FILE
// ============================================================
void muatData() {
    auto start = steady_clock::now();
    database.clear();
    ifstream file(FILE_NAME);
    if (!file.is_open()) {
        cout << ">> File \"" << FILE_NAME << "\" tidak ditemukan. Database kosong.\n";
        return;
    }
    string baris;
    getline(file, baris); // skip header
    int skipped = 0;
    while (getline(file, baris)) {
        if (baris.empty()) continue;
        vector<string> f = parseCSVLine(baris);
        if (f.size() < 6) { skipped++; continue; } 
        DataArsip d;
        d.id_dokumen     = f[0];
        d.nama_file      = f[1];
        try { d.ukuran_data = stoll(f[2]); } catch (...) { d.ukuran_data = 0; }
        d.tanggal_unggah = f[3];
        d.sumber_data    = f[4];
        d.konten         = f[5];
        database.push_back(d);
    }
    file.close();
    auto stop = steady_clock::now();
    auto dur  = duration_cast<milliseconds>(stop - start);
    cout << ">> Berhasil memuat " << database.size() << " data dalam "
         << dur.count() << " ms";
    if (skipped) cout << " (" << skipped << " baris dilewati)";
    cout << ".\n";
}

// ============================================================
//  INSERT MANUAL
// ============================================================
void insertManual() {
    DataArsip d;
    d.id_dokumen = generateNextID();
    cout << "\n[Insert Manual] ID Otomatis: " << d.id_dokumen << "\n";
    cout << "Nama File       : "; cin >> d.nama_file;
    cout << "Ukuran (bytes)  : "; cin >> d.ukuran_data;
    cout << "Sumber Data     : "; cin >> d.sumber_data;
    cout << "Tanggal Unggah  : "; cin >> d.tanggal_unggah;
    cout << "Konten          : "; cin.ignore(); getline(cin, d.konten);

    auto start = steady_clock::now();
    int idxDup = cariDuplikatMetadata(d.nama_file, d.ukuran_data);
    auto stop  = steady_clock::now();
    auto dur   = duration_cast<microseconds>(stop - start);

    if (idxDup != -1) {
        cout << ">> DITOLAK: Duplikat terdeteksi dengan ID \""
             << database[idxDup].id_dokumen << "\" "
             << "(nama + ukuran identik).\n";
        cout << ">> Waktu cek duplikat: " << dur.count() << " mikrodetik.\n";
    } else {
        database.push_back(d);
        simpanKeFile();
        cout << ">> DITERIMA: Data berhasil ditambahkan.\n";
        cout << ">> Waktu cek duplikat: " << dur.count() << " mikrodetik.\n";
    }
}

// ============================================================
//  BATCH IMPORT DARI CSV
// ============================================================
void insertBatch() {
    string fileBaru;
    cout << "\n[Batch Import] Nama file CSV: ";
    cin >> fileBaru;

    ifstream file(fileBaru);
    if (!file.is_open()) {
        cout << ">> Gagal membuka \"" << fileBaru << "\"!\n";
        return;
    }

    auto start = steady_clock::now();
    string baris;
    getline(file, baris); // skip header

    int sukses = 0, duplikat = 0, rusak = 0;
    while (getline(file, baris)) {
        if (baris.empty()) continue;
        vector<string> f = parseCSVLine(baris);
        if (f.size() < 6) { rusak++; continue; }

        DataArsip d;
        // Pakai ID dari file; jika kosong, generate baru
        d.id_dokumen     = f[0].empty() ? generateNextID() : f[0];
        d.nama_file      = f[1];
        try { d.ukuran_data = stoll(f[2]); } catch (...) { d.ukuran_data = 0; }
        d.tanggal_unggah = f[3];
        d.sumber_data    = f[4];
        d.konten         = f[5];

        if (cariDuplikatMetadata(d.nama_file, d.ukuran_data) != -1) {
            duplikat++;
        } else {
            database.push_back(d);
            sukses++;
        }
    }
    file.close();
    simpanKeFile();

    auto stop = steady_clock::now();
    auto dur  = duration_cast<milliseconds>(stop - start);
    cout << ">> Batch Import selesai dalam " << dur.count() << " ms.\n";
    cout << ">> " << sukses    << " data baru ditambahkan.\n";
    cout << ">> " << duplikat  << " data ditolak (duplikat metadata).\n";
    if (rusak) cout << ">> " << rusak << " baris dilewati (format rusak).\n";
}

// ============================================================
//  SEARCH (by ID atau nama file)
// ============================================================
void searchData() {
    string query;
    cout << "\n[Search] Masukkan ID atau Nama File: "; cin >> query;

    auto start = steady_clock::now();
    int count = 0;
    for (const auto& d : database) {
        if (d.id_dokumen == query || d.nama_file == query) {
            cout << "  ID     : " << d.id_dokumen     << "\n"
                 << "  File   : " << d.nama_file      << "\n"
                 << "  Ukuran : " << d.ukuran_data    << " bytes\n"
                 << "  Tanggal: " << d.tanggal_unggah << "\n"
                 << "  Sumber : " << d.sumber_data    << "\n"
                 << "  Konten : " << d.konten         << "\n"
                 << "  -------\n";
            count++;
        }
    }
    auto stop = steady_clock::now();
    auto dur  = duration_cast<microseconds>(stop - start);
    if (count == 0)
        cout << ">> Data tidak ditemukan.\n";
    else
        cout << ">> " << count << " record ditemukan.\n";
    cout << ">> Waktu pencarian: " << dur.count() << " mikrodetik.\n";
}

// ============================================================
//  LIST DUPLIKAT — O(n²) berbasis metadata
//  FIX: marked[] kini menandai SEMUA anggota grup (termasuk asli)
//       sehingga satu grup tidak diproses dua kali
// ============================================================
void listDuplikat() {
    cout << "\n--- Memindai Duplikat berbasis Metadata (O(n²)) ---\n";
    auto start = steady_clock::now();

    int total = (int)database.size();
    vector<bool> sudahDicetak(total, false);
    int grupDuplikat = 0;
    int totalRecordDuplikat = 0;

    for (int i = 0; i < total; i++) {
        if (sudahDicetak[i]) continue;

        vector<int> grup = {i};
        for (int j = i + 1; j < total; j++) {
            if (!sudahDicetak[j] &&
                database[i].nama_file   == database[j].nama_file &&
                database[i].ukuran_data == database[j].ukuran_data) {
                grup.push_back(j);
            }
        }

        if (grup.size() > 1) {
            grupDuplikat++;
            cout << "\n[Grup " << grupDuplikat << "] "
                 << database[i].nama_file << " | "
                 << database[i].ukuran_data << " bytes"
                 << " (" << grup.size() << " salinan)\n";
            for (int idx : grup) {
                cout << "  -> ID: " << database[idx].id_dokumen
                     << " | Sumber: " << database[idx].sumber_data
                     << " | Tanggal: " << database[idx].tanggal_unggah << "\n";
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
        cout << "\n>> " << grupDuplikat << " grup duplikat ditemukan ("
             << totalRecordDuplikat << " record terlibat).\n";
    cout << ">> Waktu pemindaian: " << dur.count() << " ms.\n";
}

// ============================================================
//  UPDATE / DELETE
// ============================================================
void updateDeleteData() {
    string id;
    cout << "\n[Update/Delete] Masukkan ID Dokumen: "; cin >> id;

    auto start = steady_clock::now();
    for (size_t i = 0; i < database.size(); i++) {
        if (database[i].id_dokumen == id) {
            cout << ">> Ditemukan: " << database[i].nama_file
                 << " (" << database[i].ukuran_data << " bytes)\n";
            cout << "1. Update Nama File\n"
                 << "2. Update Sumber Data\n"
                 << "3. Delete Data\n"
                 << "Pilih: ";
            int opsi; cin >> opsi;

            if (opsi == 1) {
                string namaBaru;
                cout << "Nama baru: "; cin >> namaBaru;
                bool konflik = false;
                for (size_t k = 0; k < database.size(); k++) {
                    if (k != i &&
                        database[k].nama_file   == namaBaru &&
                        database[k].ukuran_data == database[i].ukuran_data) {
                        konflik = true; break;
                    }
                }
                if (konflik) {
                    cout << ">> DITOLAK: Nama baru + ukuran yang sama sudah ada (duplikat metadata).\n";
                } else {
                    database[i].nama_file = namaBaru;
                    simpanKeFile();
                    cout << ">> Nama file berhasil diupdate.\n";
                }
            } else if (opsi == 2) {
                cout << "Sumber baru: "; cin >> database[i].sumber_data;
                simpanKeFile();
                cout << ">> Sumber data berhasil diupdate.\n";
            } else if (opsi == 3) {
                database.erase(database.begin() + i);
                simpanKeFile();
                cout << ">> Data berhasil dihapus "
                     << "(elemen di sebelah kanan digeser — O(n)).\n";
            } else {
                cout << ">> Opsi tidak valid.\n";
            }

            auto stop = steady_clock::now();
            cout << ">> Waktu operasi: "
                 << duration_cast<microseconds>(stop - start).count()
                 << " mikrodetik.\n";
            return;
        }
    }
    cout << ">> ID tidak ditemukan.\n";
}

// ============================================================
//  STATISTIK
//  FIX: hitung grup duplikat dan total record duplikat secara terpisah
// ============================================================
void tampilkanStatistik() {
    cout << "\n========== STATISTIK DATABASE ==========\n";
    auto start = steady_clock::now();

    int total = (int)database.size();
    vector<bool> sudahDihitung(total, false);
    int grupDuplikat       = 0;
    int totalRecordDuplikat = 0;

    for (int i = 0; i < total; i++) {
        if (sudahDihitung[i]) continue;
        vector<int> grup = {i};
        for (int j = i + 1; j < total; j++) {
            if (!sudahDihitung[j] &&
                database[i].nama_file   == database[j].nama_file &&
                database[i].ukuran_data == database[j].ukuran_data) {
                grup.push_back(j);
            }
        }
        if (grup.size() > 1) {
            grupDuplikat++;
            totalRecordDuplikat += (int)grup.size(); // semua salinan, termasuk asli
        }
        for (int idx : grup) sudahDihitung[idx] = true;
    }

    int recordUnik = total - totalRecordDuplikat;

    auto stop = steady_clock::now();
    auto dur  = duration_cast<milliseconds>(stop - start);

    cout << "Total Record di Sistem  : " << total                 << "\n";
    cout << "Record Benar-benar Unik : " << recordUnik            << "\n";
    cout << "Grup Duplikat           : " << grupDuplikat          << "\n";
    cout << "Record Terlibat Duplikat: " << totalRecordDuplikat   << "\n";
    cout << "Waktu Kalkulasi         : " << dur.count()           << " ms (O(n²))\n";
    cout << "=========================================\n";
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    cout << "============================================\n";
    cout << "  SISTEM DETEKSI DUPLIKASI ARSIP — VECTOR  \n";
    cout << "  Deteksi berbasis: nama_file + ukuran_data\n";
    cout << "============================================\n";
    muatData();

    int menu;
    do {
        cout << "\n--- MENU UTAMA ---\n"
             << "1. Insert Manual\n"
             << "2. Batch Import (CSV)\n"
             << "3. Cari Data (Search)\n"
             << "4. Tampilkan Daftar Duplikat\n"
             << "5. Update / Delete Data\n"
             << "6. Lihat Statistik\n"
             << "7. Keluar\n"
             << "Pilih (1-7): ";
        cin >> menu;
        switch (menu) {
            case 1: insertManual();       break;
            case 2: insertBatch();        break;
            case 3: searchData();         break;
            case 4: listDuplikat();       break;
            case 5: updateDeleteData();   break;
            case 6: tampilkanStatistik(); break;
            case 7: cout << "Program ditutup.\n"; break;
            default: cout << ">> Opsi tidak valid.\n";
        }
    } while (menu != 7);
    return 0;
}
