================================================================
  DATASET: Sistem Deteksi Duplikasi Data - Arsip Digital
  Mata Kuliah: Struktur Data, Semester Genap 2025-2026
================================================================

DESKRIPSI
---------
Dataset simulasi arsip digital dengan dokumen dari berbagai
sumber. Setiap dataset mengandung 10% record duplikat yang
disebar secara acak di antara record unik.

FILE YANG TERSEDIA
------------------
  arsip_0001000.csv   →    1.000 record  (  900 unik +   100 duplikat)
  arsip_0005000.csv   →    5.000 record  (4.500 unik +   500 duplikat)
  arsip_0010000.csv   →   10.000 record  (9.000 unik + 1.000 duplikat)
  arsip_0050000.csv   →   50.000 record  (45.000 unik + 5.000 duplikat)
  arsip_0100000.csv   →  100.000 record  (90.000 unik + 10.000 duplikat)

STRUKTUR KOLOM
--------------
  id              : Identifier unik dokumen
                    Format DOC-XXXXXXX = record unik
                    Format DUP-XXXXXXX = record duplikat
  nama_file       : Nama file dengan pola: kategori_dept_YYYYMM_seq.ext
  ukuran_bytes    : Ukuran file dalam bytes 
  tanggal_unggah  : Tanggal unggah (YYYY-MM-DD), rentang 2022-2024
  sumber          : Nama server/storage sumber file
  konten          : Representasi ringkas isi: kategori|dept|ukuran|keywords

LOGIKA DUPLIKASI
----------------
Record duplikat memiliki ID berbeda (DUP-*) namun kolom
nama_file, ukuran_bytes, tanggal_unggah, sumber, dan konten
IDENTIK dengan record aslinya. Ini mensimulasikan kondisi nyata
di mana file yang sama diunggah ulang dengan identifier baru.

CARA DETEKSI DUPLIKAT
---------------------
  - Berbasis konten : bandingkan field 'konten'
  - Berbasis metadata: gabungkan nama_file + ukuran_bytes
  - Berbasis hash key: hash(nama_file + "_" + ukuran_bytes)

GENERATOR
---------
  Script : generate_dataset.py
  Seed   : 42 (reproducible)
  Bahasa : Python 3
================================================================
