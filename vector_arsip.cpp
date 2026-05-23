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
    for (char c : baris) {
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
    oss << "DOC-" << setw(7) << setfill('0') << (maxNum + 1);
    return oss.str();
}

// ============================================================
//  UTILITAS: cek duplikat berbasis metadata (nama_file + ukuran_data)
//  Kembalikan indeks record pertama yang cocok, -1 jika tidak ada
// ============================================================
int cariDuplikatMetadata(const string& nama, long long ukuran) {
    for (size_t i = 0; i < database.size(); i++) {
        if (database[i].nama_file == nama && database[i].ukuran_data == ukuran)
            return (int)i;
    }
    return -1;
}

// ============================================================
//  UTILITAS VALIDASI
// ============================================================

// Ekstensi yang diizinkan di sistem arsip digital
const vector<string> EKSTENSI_VALID = {
    "pdf", "docx", "doc", "xlsx", "xls",
    "txt", "csv", "json", "xml",
    "png", "jpg", "jpeg", "zip", "rar"
};

// Nama file: tidak boleh kosong, tidak boleh karakter ilegal, dan harus berekstensi dari whitelist
bool validasiNamaFile(const string& nama) {
    if (nama.empty()) return false;
    for (char c : nama)
        if (c == ',' || c == '"' || c == '\\' || c == '/') return false;

    // Cari posisi titik terakhir
    size_t titik = nama.rfind('.');
    if (titik == string::npos || titik == nama.size() - 1) return false; // tidak ada ekstensi

    string ext = nama.substr(titik + 1);
    // Ubah ke lowercase untuk perbandingan
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    for (const auto& e : EKSTENSI_VALID)
        if (ext == e) return true;
    return false;
}

// Tampilkan daftar ekstensi yang valid (dipanggil saat error)
void tampilkanEkstensiValid() {
    cout << "   [!] Ekstensi tidak diizinkan. Ekstensi yang valid:\n       ";
    for (size_t i = 0; i < EKSTENSI_VALID.size(); i++) {
        cout << "." << EKSTENSI_VALID[i];
        if (i < EKSTENSI_VALID.size() - 1) cout << "  ";
    }
    cout << "\n";
}

// Tanggal: harus format YYYY-MM-DD dan range masuk akal
bool validasiTanggal(const string& tgl) {
    if (tgl.size() != 10) return false;
    if (tgl[4] != '-' || tgl[7] != '-') return false;
    try {
        int y = stoi(tgl.substr(0, 4));
        int m = stoi(tgl.substr(5, 2));
        int d = stoi(tgl.substr(8, 2));
        if (y < 2000 || y > 2100) return false;
        if (m < 1 || m > 12)      return false;
        if (d < 1 || d > 31)      return false;
    } catch (...) { return false; }
    return true;
}

// Sumber: tidak boleh kosong
bool validasiSumber(const string& sumber) {
    return !sumber.empty();
}

// ============================================================
//  GENERATE KONTEN OTOMATIS
//  Format: namatanpaekstensi|ukuran|keyword
//  Contoh: laporan_keuangan_202401|512000|rekap final
// ============================================================
string generateKonten(const string& namaFile, long long ukuran,
                      const string& keywords) {
    // Ambil nama file tanpa ekstensi
    size_t titik = namaFile.rfind('.');
    string namaBersih = (titik != string::npos) ? namaFile.substr(0, titik) : namaFile;

    string konten = namaBersih + "|" + to_string(ukuran);
    if (!keywords.empty()) konten += "|" + keywords;
    return konten;
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
//  INSERT MANUAL ( Validasi tiap field satu per satu )
// ============================================================
void insertManual() {
    DataArsip d;
    d.id_dokumen = generateNextID();
    cout << "\n[Insert Manual] ID Otomatis: " << d.id_dokumen << "\n";

    // --- Nama File ---
    do {
        cout << "Nama File       : ";
        cin >> d.nama_file;
        if (!validasiNamaFile(d.nama_file)) {
            // Cek apakah masalahnya di karakter ilegal atau di ekstensi
            bool adaKarIlegal = false;
            for (char c : d.nama_file)
                if (c == ',' || c == '"' || c == '\\' || c == '/')
                    { adaKarIlegal = true; break; }
            if (adaKarIlegal)
                cout << "   [!] Nama file mengandung karakter tidak valid ( , \" \\ / ).\n";
            else
                tampilkanEkstensiValid();
        }
    } while (!validasiNamaFile(d.nama_file));
    
    // --- Ukuran ---
    while (true) {
        cout << "Ukuran (bytes)  : ";
        if (cin >> d.ukuran_data && d.ukuran_data > 0) break;
        cout << "   [!] Ukuran harus angka positif lebih dari 0.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // --- Sumber Data ---
    do {
        cout << "Sumber Data     : ";
        cin >> d.sumber_data;
        if (!validasiSumber(d.sumber_data))
            cout << "   [!] Sumber data tidak boleh kosong.\n";
    } while (!validasiSumber(d.sumber_data));

    // --- Tanggal ---
    do {
        cout << "Tanggal Unggah (YYYY-MM-DD) : ";
        cin >> d.tanggal_unggah;
        if (!validasiTanggal(d.tanggal_unggah))
            cout << "   [!] Format tanggal tidak valid. Gunakan YYYY-MM-DD (contoh: 2024-03-15).\n";
    } while (!validasiTanggal(d.tanggal_unggah));

    // --- Konten (auto-generate + keyword opsional) ---
    cin.ignore();
    cout << "Keyword Tambahan: ";
    cout << "(opsional, pisahkan spasi, Enter untuk lewati)\n";
    cout << "                : ";
    string keywords;
    getline(cin, keywords);
    d.konten = generateKonten(d.nama_file, d.ukuran_data, keywords);
    cout << "   >> Konten    : " << d.konten << "\n";

    // --- Cek Duplikat ---
    auto start = steady_clock::now();
    int idxDup = cariDuplikatMetadata(d.nama_file, d.ukuran_data);
    auto stop  = steady_clock::now();
    auto dur   = duration_cast<microseconds>(stop - start);

    if (idxDup != -1) {
        cout << ">> DITOLAK: Duplikat terdeteksi dengan ID \""
             << database[idxDup].id_dokumen << "\" "
             << "(nama + ukuran identik).\n";
    } else {
        database.push_back(d);
        simpanKeFile();
        cout << ">> DITERIMA: Data berhasil ditambahkan.\n";
    }
    cout << ">> Waktu cek duplikat: " << dur.count() << " mikrodetik.\n";
}

// ============================================================
//  BATCH IMPORT DARI CSV
//
//  Alur tiap baris:
//    1. Format rusak (field < 6)      → rejected.csv  (alasan: format_rusak)
//    2. Field tidak lolos validasi    → rejected.csv  (alasan: detail field)
//    3. Duplikat metadata             → TETAP masuk DB, dicatat sebagai duplikat
//    4. Valid & unik                  → masuk DB
//
//  ID dari file CSV diabaikan, selalu generate DOC- baru
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

    // Siapkan file rejected
    string fileRejected = "rejected_" + fileBaru;
    ofstream rejFile(fileRejected);
    rejFile << "baris_ke,nama_file,ukuran_data,tanggal_unggah,sumber_data,konten,alasan_tolak\n";
    bool adaRejected = false;

    auto start = steady_clock::now();
    string baris;
    getline(file, baris); // skip header

    int noBaris  = 1;
    int sukses   = 0;
    int duplikat = 0;
    int ditolak  = 0;

    while (getline(file, baris)) {
        if (baris.empty()) continue;
        noBaris++;

        vector<string> f = parseCSVLine(baris);

        // --- Cek 1: field tidak lengkap ---
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

        // Coba parse ukuran
        try { ukuran = stoll(ukuStr); } catch (...) { ukuranValid = false; }

        // --- Cek 2: validasi field per field ---
        string alasan = "";
        if (!validasiNamaFile(nama)) {
            // Bedakan alasan: karakter ilegal vs ekstensi tidak valid
            bool adaKarIlegal = false;
            for (char c : nama)
                if (c == ',' || c == '"' || c == '\\' || c == '/')
                    { adaKarIlegal = true; break; }
            alasan += adaKarIlegal ? "nama_file_karakter_ilegal;" : "nama_file_ekstensi_tidak_valid;";
        }
        if (!ukuranValid || ukuran <= 0)
            alasan += "ukuran_tidak_valid;";
        if (!validasiTanggal(tgl))
            alasan += "tanggal_tidak_valid;";
        if (!validasiSumber(sumber))
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

        // --- Cek 3: duplikat metadata — TETAP masuk DB ---
        DataArsip d;
        d.id_dokumen     = generateNextID(); // selalu generate baru, abaikan ID dari file
        d.nama_file      = nama;
        d.ukuran_data    = ukuran;
        d.tanggal_unggah = tgl;
        d.sumber_data    = sumber;
        d.konten         = konten;

        if (cariDuplikatMetadata(d.nama_file, d.ukuran_data) != -1) {
            database.push_back(d); // duplikat tetap masuk, ini yang ingin kita deteksi
            duplikat++;
        } else {
            database.push_back(d);
            sukses++;
        }
    }

    file.close();
    rejFile.close();
    if (!adaRejected) remove(fileRejected.c_str()); // hapus jika kosong

    simpanKeFile();

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
                // Validasi nama baru
                do {
                    cout << "Nama baru: "; cin >> namaBaru;
                    if (!validasiNamaFile(namaBaru))
                        cout << "   [!] Nama file tidak valid.\n";
                } while (!validasiNamaFile(namaBaru));

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
                string sumberBaru;
                do {
                    cout << "Sumber baru: "; cin >> sumberBaru;
                    if (!validasiSumber(sumberBaru))
                        cout << "   [!] Sumber tidak boleh kosong.\n";
                } while (!validasiSumber(sumberBaru));
                database[i].sumber_data = sumberBaru;
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