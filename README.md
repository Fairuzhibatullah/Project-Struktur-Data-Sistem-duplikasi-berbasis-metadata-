# Sistem Deteksi Duplikasi Data — Arsip Digital
**Proyek Akhir Mata Kuliah Struktur Data | Semester Genap 2025-2026**

---

## Deskripsi Proyek

Sistem ini mensimulasikan deteksi duplikasi data pada lingkungan arsip digital. Data dari berbagai sumber dikumpulkan secara berkala, sehingga duplikasi sangat mungkin terjadi. Proyek ini membangun dan membandingkan performa dua struktur data — **Vector** dan **Hash Table** — dalam menangani operasi insert, search, deteksi duplikat, update, dan delete.

Deteksi duplikasi dilakukan secara sederhana berbasis perbandingan metadata (`nama_file + ukuran_data`), tanpa algoritma hashing kriptografi atau machine learning.

---

## Struktur Repository

```
├── README.md                   ← Dokumen ini
├── vector_arsip.cpp            ← Implementasi dengan std::vector (Engine 1)
├── hash_arsip.cpp              ← Implementasi dengan unordered_map (Engine 2) [WIP]
└── datasets/
    ├── arsip_0001000.csv       ←   1.000 record (900 unik + 100 duplikat)
    ├── arsip_0005000.csv       ←   5.000 record (4.500 unik + 500 duplikat)
    ├── arsip_0010000.csv       ←  10.000 record (9.000 unik + 1.000 duplikat)
    ├── arsip_0050000.csv       ←  50.000 record (45.000 unik + 5.000 duplikat)
    └── arsip_0100000.csv       ← 100.000 record (90.000 unik + 10.000 duplikat)
```

---

## Domain Data

Setiap record merepresentasikan satu dokumen dalam arsip digital dengan field:

| Field | Tipe | Contoh | Keterangan |
|---|---|---|---|
| `id_dokumen` | string | `DOC-0000001` | ID unik; prefix `DUP-` = duplikat |
| `nama_file` | string | `laporan_keuangan_202309_0042.pdf` | Nama file dengan pola: kategori_dept_YYYYMM_seq.ext |
| `ukuran_data` | long long | `392275674` | Ukuran file dalam bytes |
| `tanggal_unggah` | string | `2023-09-14` | Format YYYY-MM-DD |
| `sumber_data` | string | `server-arsip-jakarta` | Nama server/storage asal |
| `konten` | string | `laporan\|keuangan\|392275674\|draft rekap` | Representasi ringkas isi |

---

## Dataset

Dataset di-generate menggunakan `generate_dataset.py` dengan ketentuan:

- **Seed**: 42 (reproducible — hasil generate selalu sama)
- **Rasio duplikat**: 10% dari total record
- **Distribusi ukuran file**: realistis (mayoritas 10KB–5MB, sebagian kecil hingga 500MB)
- **Rentang tanggal**: 2022–2024
- **Cara duplikasi**: record duplikat memiliki ID baru (`DUP-`) tetapi semua field lainnya identik dengan record aslinya, mensimulasikan file yang diunggah ulang


## Implementasi Struktur Data

### Engine 1 — Vector (`vector_arsip.cpp`)

Menggunakan `std::vector<DataArsip>` sebagai kontainer utama.

**Kompleksitas Operasi:**

| Operasi | Kompleksitas | Catatan |
|---|---|---|
| Insert | O(n) | Cek duplikat dulu dengan linear scan |
| Search by ID/nama | O(n) | Linear scan seluruh vector |
| Deteksi duplikat | O(n²) | Loop ganda membandingkan setiap pasang record |
| Delete | O(n) | Erase + geser elemen di sebelah kanan |
| Load dari file | O(n) | Baca baris per baris |

**Fitur:**
- Insert manual (ID otomatis format `MAN-XXXXXXX`)
- Batch import dari file CSV
- Search berdasarkan ID atau nama file
- Tampilkan seluruh grup duplikat beserta anggotanya
- Update nama file / sumber data (dengan validasi duplikat)
- Delete record by ID
- Statistik: total record, unik, grup duplikat, record terlibat duplikat

**Cara kompilasi & jalankan:**
```bash
g++ -O2 -std=c++17 -o vector_arsip vector_arsip.cpp
./vector_arsip
```

> Ubah konstanta `FILE_NAME` di baris atas file untuk mengganti dataset yang digunakan.

---

### Engine 2 — Hash Table (`hash_arsip.cpp`) `[WIP]`

Menggunakan `std::unordered_map` dan `std::unordered_set`.

**Kompleksitas Operasi (target):**

| Operasi | Kompleksitas | Catatan |
|---|---|---|
| Insert | O(1) avg | Hash key = nama_file + "_" + ukuran_data |
| Search by ID/nama | O(1) avg | Direct lookup via hash |
| Deteksi duplikat | O(1) avg | Cek eksistensi key di unordered_set |
| Delete | O(1) avg | Hash remove |
| Load dari file | O(n) | Baca + insert satu per satu |

---

## Rencana Eksperimen Perbandingan

Kedua engine akan diuji pada semua ukuran dataset (1K, 5K, 10K, 50K, 100K record) untuk mengukur:

1. **Waktu deteksi duplikat** — metrik utama (Vector O(n²) vs Hash O(n))
2. **Waktu search** — O(n) vs O(1)
3. **Waktu batch import** — O(n²) vs O(n)
4. **Penggunaan memori** — overhead vector vs hash table

Hasil akan disajikan dalam grafik perbandingan waktu eksekusi vs jumlah record.

---

## Target Progress

| Minggu | Target | Status |
|---|---|---|
| 7 | Spesifikasi sistem, dataset dummy, implementasi ≥1 struktur data, uji insert & deteksi | ✅ Selesai |
| 14 | Implementasi ≥2 struktur data, grafik perbandingan, demo, laporan analisis | 🔄 In progress |

---

## Referensi Dataset

Dataset di-generate secara mandiri menggunakan script Python (`generate_dataset.py`) dengan data sintetis yang merepresentasikan skenario arsip digital nyata. Tidak menggunakan dataset publik eksternal.
