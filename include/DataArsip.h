#ifndef DATA_ARSIP_H
#define DATA_ARSIP_H

#include <string>

struct DataArsip {
    std::string    id_dokumen;
    std::string    nama_file;
    long long      ukuran_data = 0;
    std::string    tanggal_unggah;
    std::string    sumber_data;
    std::string    konten;
};

#endif
