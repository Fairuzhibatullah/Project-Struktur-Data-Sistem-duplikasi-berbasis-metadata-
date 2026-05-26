#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

class Validator {
public:
    static inline std::vector<std::string> parseCSVLine(const std::string& baris) {
        std::vector<std::string> fields;
        std::string field;
        bool inQuotes = false;
        for (char c : baris) {
            if (c == '"')                inQuotes = !inQuotes;
            else if (c == ',' && !inQuotes) { fields.push_back(field); field.clear(); }
            else                            field += c;
        }
        fields.push_back(field);
        return fields;
    }

    static inline bool validasiNamaFile(const std::string& nama) {
        if (nama.empty()) return false;
        for (char c : nama)
            if (c == ',' || c == '"' || c == '\\' || c == '/') return false;

        size_t titik = nama.rfind('.');
        if (titik == std::string::npos || titik == nama.size() - 1) return false;

        std::string ext = nama.substr(titik + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        const std::vector<std::string> EKSTENSI_VALID = {
            "pdf", "docx", "doc", "xlsx", "xls",
            "txt", "csv", "json", "xml",
            "png", "jpg", "jpeg", "zip", "rar"
        };

        for (const auto& e : EKSTENSI_VALID)
            if (ext == e) return true;
        return false;
    }

    static inline void tampilkanEkstensiValid() {
        const std::vector<std::string> EKSTENSI_VALID = {
            "pdf", "docx", "doc", "xlsx", "xls",
            "txt", "csv", "json", "xml",
            "png", "jpg", "jpeg", "zip", "rar"
        };
        std::cout << "   [!] Ekstensi tidak diizinkan. Ekstensi yang valid:\n       ";
        for (size_t i = 0; i < EKSTENSI_VALID.size(); i++) {
            std::cout << "." << EKSTENSI_VALID[i];
            if (i < EKSTENSI_VALID.size() - 1) std::cout << "  ";
        }
        std::cout << "\n";
    }

    static inline bool validasiTanggal(const std::string& tgl) {
        if (tgl.size() != 10) return false;
        if (tgl[4] != '-' || tgl[7] != '-') return false;
        for (int i = 0; i < 10; i++) {
            if (i == 4 || i == 7) continue;
            if (!isdigit(tgl[i])) return false;
        }
        return true;
    }

    static inline bool validasiSumber(const std::string& sumber) {
        return !sumber.empty();
    }

    static inline std::string generateKonten(const std::string& namaFile, long long ukuran,
                                            const std::string& keywords) {
        size_t titik = namaFile.rfind('.');
        std::string namaBersih = (titik != std::string::npos) ? namaFile.substr(0, titik) : namaFile;

        std::string konten = namaBersih + "|" + std::to_string(ukuran);
        if (!keywords.empty()) konten += "|" + keywords;
        return konten;
    }
};

#endif 
