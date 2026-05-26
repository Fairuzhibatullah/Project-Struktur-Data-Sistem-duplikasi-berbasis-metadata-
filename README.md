# Sistem Deteksi Duplikasi Data — Arsip Digital
**Proyek Akhir Mata Kuliah Struktur Data C++**

---

##  Deskripsi Proyek

Sistem ini mensimulasikan deteksi duplikasi data pada lingkungan arsip digital skala industri. Proyek ini mengimplementasikan dan membandingkan performa dua struktur data utama — **std::vector** (Engine 1) dan **Hash Table / std::unordered_map** (Engine 2) — dalam menangani operasi penyimpanan data (*insert*), pencarian (*search*), pemindaian duplikat (*duplicate scan*), pembaharuan (*update*), dan penghapusan (*delete*).

Deteksi duplikasi data didukung oleh **dua mode deteksi** yang berjalan secara berdampingan:
1. **Metadata-Based**: Membandingkan kecocokan atribut `nama_file + ukuran_data` ($O(1)$ untuk Hash Table, $O(n)$ untuk Vector).
2. **Content-Based**: Membandingkan isi representasi konten dokumen (`konten`) untuk menangani berkas identik dengan nama berbeda.

---

## 📂 Struktur Repositori

Proyek menggunakan arsitektur **OOP Polymorphism** dan **Separation of Concerns (SoC)**. Seluruh file pustaka C++ dikelompokkan ke dalam folder `include/` untuk menghindari penumpukan berkas pada direktori utama:

```
├── include/
│   ├── DataArsip.h      ← [Model] Struktur data representasi dokumen
│   ├── Validator.h      ← [Utility] Logika validasi input & parser CSV
│   ├── BaseEngine.h     ← [Abstract Interface] Kontrak virtual murni operasi data
│   ├── VectorEngine.h   ← [Engine 1] Implementasi database berbasis std::vector (O(n²))
│   ├── HashEngine.h     ← [Engine 2] Implementasi database berbasis Hash Table (O(n))
│   └── ArsipManager.h   ← [Controller/View] Logika Menu CLI & antarmuka interaktif
│
├── datasets/
│   ├── arsip_0001000.csv       ←   1.000 record (900 unik + 100 duplikat)
│   ├── arsip_0005000.csv       ←   5.000 record (4.500 unik + 500 duplikat)
│   ├── arsip_0010000.csv       ←  10.000 record (9.000 unik + 1.000 duplikat)
│   ├── arsip_0050000.csv       ←  50.000 record (45.000 unik + 5.000 duplikat)
│   └── arsip_0100000.csv       ← 100.000 record (90.000 unik + 10.000 duplikat)
│
├── README.md                   ← Dokumen analisis proyek
├── statistik_log.csv           ← File log perekaman performa eksekusi uji coba
├── vector_database.csv         ← Database lokal fisik Engine Vector
├── hash_database.csv           ← Database lokal fisik Engine Hash Table
│
├── vector_arsip.cpp            ← Titik masuk program Engine Vector
└── hash_arsip.cpp              ← Titik masuk program Engine Hash Table
```

---

## 📊 Domain Data & Metadata

Setiap dokumen di dalam sistem direpresentasikan oleh atribut terstruktur sebagai berikut:

| Bidang (Field) | Tipe | Contoh Nilai | Deskripsi Atribut |
|---|---|---|---|
| `id_dokumen` | string | `DOC-0000001` | ID Dokumen unik (dihasilkan secara otomatis oleh sistem) |
| `nama_file` | string | `laporan_keuangan_2023.pdf` | Nama berkas dengan ekstensi yang tervalidasi |
| `ukuran_data` | long long | `392275674` | Ukuran berkas dalam satuan byte |
| `tanggal_unggah` | string | `2026-05-26` | Tanggal dokumen diunggah (Format: `YYYY-MM-DD`) |
| `sumber_data` | string | `server-jakarta` | Nama server/sumber penyimpanan asal berkas |
| `konten` | string | `laporan_keuangan 392275674` | Representasi string pendek ringkasan isi dokumen |

---

## ⚙️ Perbandingan Kompleksitas Teoretis

| Operasi Sistem | Kompleksitas Vector Engine | Kompleksitas Hash Engine | Deskripsi Teknis |
|---|---|---|---|
| **Generator ID** | $O(1)$ *(Optimized)* | $O(1)$ | Menggunakan pelacakan global `maxIDNum` tanpa linier scan. |
| **Insert Record** | $O(n)$ | $O(1)$ rata-rata | Cek bentrokan duplikat instan di Hash Table sebelum ditulis. |
| **Search by ID** | $O(n)$ linier scan | $O(1)$ rata-rata | *Direct mapping lookup* menggunakan `std::unordered_map`. |
| **Search by Name** | $O(n)$ linier scan | $O(1)$ rata-rata | Memanfaatkan indeks bantu `indexNamaFile` secara instan. |
| **Duplicate Scan** | $O(n^2)$ double loop | $O(n)$ linier | Iterasi langsung pada indeks map yang beranggota $> 1$. |
| **Delete (Hapus)** | $O(n)$ geser memori | $O(1)$ rata-rata | Penghapusan kunci hash tanpa operasi pergeseran alokasi memory. |
| **Simpan ke File** | $O(n)$ | $O(n)$ | Serialisasi data terurut ke format CSV fisik. |

---

## 🚀 Panduan Kompilasi & Menjalankan

Karena seluruh pustaka implementasi dipisahkan secara modular di dalam folder `include/`, proses kompilasi tetap sangat sederhana karena Anda cukup memanggil berkas `.cpp` utamanya saja:

### 1. Kompilasi Program (Windows GCC / G++):
```bash
# Kompilasi Engine Vector
g++ -O2 -std=c++17 -o vector_arsip vector_arsip.cpp

# Kompilasi Engine Hash Table
g++ -O2 -std=c++17 -o hash_arsip hash_arsip.cpp
```

### 2. Menjalankan Program:
```bash
# Menjalankan Engine Vector
./vector_arsip

# Menjalankan Engine Hash Table
./hash_arsip
```

---

## 📝 Pengumpulan Data Eksperimen

1. Jalankan kedua program di atas secara berurutan.
2. Lakukan impor dataset yang sama dari subfolder `datasets/` (misalnya 1.000, 5.000, atau 10.000 data) melalui menu **2 (Batch Import)**.
3. Jalankan menu **6 (Lihat Statistik)** untuk memproses grup duplikat secara riil. Setiap kali menu statistik dipanggil, durasi komputasi dan ukuran record akan terekam ke file **`statistik_log.csv`**.
4. Buka file `statistik_log.csv` menggunakan **Microsoft Excel** untuk merancang tabel perbandingan performa eksekusi waktu (ms) dan menggambarkan grafik pertumbuhan waktu (O(N^2) vs O(N)).
