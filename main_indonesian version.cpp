#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

const int MAX_SLOTS = 10;
const int MAX_MATKUL_DB = 100;
const int MAX_MAHASISWA = 50;

// Struktur untuk sesi jam (sesuai SKS)
struct SesiJam {
    string jamMulai;
    string jamSelesai;
    int durasiMenit; // dalam menit
};

// Daftar sesi jam yang tersedia (07.30 - 15.30, total 10 SKS)
// 1 SKS = 45 menit, dengan istirahat di 09.45-10.00 dan 13.00-13.15
const SesiJam SESI_JAM[] = {
    // Sesi 1 SKS (45 menit)
    {"07.30", "08.15", 45},   // Slot 1
    {"08.15", "09.00", 45},   // Slot 2
    {"09.00", "09.45", 45},   // Slot 3
    {"10.00", "10.45", 45},   // Slot 4 (setelah istirahat)
    {"10.45", "11.30", 45},   // Slot 5
    {"11.30", "12.15", 45},   // Slot 6
    {"12.15", "13.00", 45},   // Slot 7
    {"13.15", "14.00", 45},   // Slot 8 (setelah istirahat)
    {"14.00", "14.45", 45},   // Slot 9
    {"14.45", "15.30", 45},   // Slot 10

    // Sesi 2 SKS (90 menit)
    {"07.30", "09.00", 90},   // Slot 1-2
    {"08.15", "09.45", 90},   // Slot 2-3
    {"10.00", "11.30", 90},   // Slot 4-5
    {"10.45", "12.15", 90},   // Slot 5-6
    {"11.30", "13.00", 90},   // Slot 6-7
    {"13.15", "14.45", 90},   // Slot 8-9
    {"14.00", "15.30", 90},   // Slot 9-10

    // Sesi 3 SKS (135 menit)
    {"07.30", "09.45", 135},  // Slot 1-3
    {"10.00", "12.15", 135},  // Slot 4-6
    {"10.45", "13.00", 135},  // Slot 5-7
    {"13.15", "15.30", 135},  // Slot 8-10
};
const int JUMLAH_SESI = 21;

// Daftar hari yang tersedia
const string HARI[] = {"Senin", "Selasa", "Rabu", "Kamis", "Jumat"};
const int JUMLAH_HARI = 5;

struct MatkulMaster {
    string namaMatkul;
    string jamMulai;
    string jamSelesai;
    string ruangan;
    int sks;
    string dosen;
    string hari; // Menyimpan hari (Senin-Jumat)
    int indexSesi; // Menyimpan index sesi yang dipilih
};

struct Mahasiswa {
    string nama;
    string npm;
    string jadwal[MAX_SLOTS + 1];
    string ruangan[MAX_SLOTS + 1];
    string jamMulai[MAX_SLOTS + 1];
    string jamSelesai[MAX_SLOTS + 1];
    string hari[MAX_SLOTS + 1]; // Menyimpan hari untuk setiap slot
};

MatkulMaster databaseMatkul[MAX_MATKUL_DB];
int jumlahMatkulTerdaftar = 0;

Mahasiswa databaseMhs[MAX_MAHASISWA];
int jumlahMhsTerdaftar = 0;

// --- FUNGSI BANTUAN ---
string trim(string str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void printHeader(string title) {
    int panjang = title.length() + 4;
    cout << "\n";
    cout << string(panjang, '=') << endl;
    cout << "  " << title << endl;
    cout << string(panjang, '=') << endl;
}

void printLine() {
    cout << "------------------------------------------------------------" << endl;
}

void printLineDynamic(int panjang) {
    cout << string(panjang, '-') << endl;
}

bool cariMatkul(string namaDicari, MatkulMaster &result) {
    for (int i = 0; i < jumlahMatkulTerdaftar; i++) {
        if (databaseMatkul[i].namaMatkul == namaDicari) {
            result = databaseMatkul[i];
            return true;
        }
    }
    return false;
}

bool cariMahasiswaByNama(string namaDicari, Mahasiswa &result) {
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        if (databaseMhs[i].nama == namaDicari) {
            result = databaseMhs[i];
            return true;
        }
    }
    return false;
}

// Fungsi untuk validasi input angka
int inputInteger(string prompt, int min = INT_MIN, int max = INT_MAX) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (cin.fail()) {
            cout << "   [!] Input harus berupa angka! Silakan coba lagi.\n" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (value < min || value > max) {
            cout << "   [!] Input harus antara " << min << " dan " << max << "! Silakan coba lagi.\n" << endl;
            continue;
        }

        return value;
    }
}

// Fungsi untuk menampilkan pilihan hari
void tampilkanPilihanHari() {
    cout << "\nPilihan Hari Kuliah:" << endl;
    cout << "-------------------" << endl;
    for (int i = 0; i < JUMLAH_HARI; i++) {
        cout << (i+1) << ". " << HARI[i] << endl;
    }
    cout << "-------------------" << endl;
    cout << "Catatan: Sabtu dan Minggu libur (tidak bisa dipilih)" << endl;
    cout << "-------------------" << endl;
}

// Fungsi untuk menampilkan pilihan sesi jam berdasarkan SKS
void tampilkanPilihanSesi(int sks) {
    cout << "\nPilihan Sesi Jam (berdasarkan " << sks << " SKS):" << endl;
    cout << "------------------------------------------------" << endl;

    int durasiDibutuhkan = sks * 45; // 1 SKS = 45 menit
    int counter = 1;

    for (int i = 0; i < JUMLAH_SESI; i++) {
        if (SESI_JAM[i].durasiMenit == durasiDibutuhkan) {
            cout << counter << ". " << SESI_JAM[i].jamMulai << " - "
                 << SESI_JAM[i].jamSelesai << " (" << sks << " SKS)" << endl;
            counter++;
        }
    }

    if (counter == 1) {
        cout << "Tidak ada sesi yang tersedia untuk " << sks << " SKS." << endl;
    }
    cout << "------------------------------------------------" << endl;
}

// Fungsi untuk mendapatkan sesi berdasarkan pilihan
bool dapatkanSesiBerdasarkanPilihan(int sks, int pilihan, SesiJam &sesi) {
    int durasiDibutuhkan = sks * 45;
    int counter = 1;

    for (int i = 0; i < JUMLAH_SESI; i++) {
        if (SESI_JAM[i].durasiMenit == durasiDibutuhkan) {
            if (counter == pilihan) {
                sesi = SESI_JAM[i];
                return true;
            }
            counter++;
        }
    }

    return false;
}

// Fungsi untuk memilih hari
string pilihHari() {
    int pilihanHari;
    
    do {
        tampilkanPilihanHari();
        pilihanHari = inputInteger("Pilih hari (1-5): ", 1, JUMLAH_HARI);
    } while (pilihanHari < 1 || pilihanHari > JUMLAH_HARI);
    
    return HARI[pilihanHari - 1];
}

// Fungsi untuk cek bentrok jadwal mahasiswa
bool cekBentrokJadwal(Mahasiswa mhs, string hariBaru, string jamMulaiBaru, string jamSelesaiBaru, string matkulYangDiCek = "") {
    // Konversi waktu ke menit untuk perbandingan
    auto waktuKeMenit = [](string waktu) {
        int jam = stoi(waktu.substr(0, 2));
        int menit = stoi(waktu.substr(3, 2));
        return jam * 60 + menit;
    };
    
    int mulaiBaru = waktuKeMenit(jamMulaiBaru);
    int selesaiBaru = waktuKeMenit(jamSelesaiBaru);
    
    // Cek setiap slot jadwal mahasiswa
    for (int s = 1; s <= MAX_SLOTS; s++) {
        if (mhs.jadwal[s] != "Kosong" && mhs.jadwal[s] != "-" && mhs.jadwal[s] != matkulYangDiCek) {
            // Cek jika hari sama
            if (mhs.hari[s] == hariBaru) {
                int mulaiAda = waktuKeMenit(mhs.jamMulai[s]);
                int selesaiAda = waktuKeMenit(mhs.jamSelesai[s]);
                
                // Cek bentrok waktu
                if ((mulaiBaru < selesaiAda && selesaiBaru > mulaiAda)) {
                    return true; // Ada bentrok
                }
            }
        }
    }
    
    return false; // Tidak ada bentrok
}

// --- FUNGSI MEMUAT DATA DARI FILE ---
void muatDataMatkul() {
    ifstream file("matkul.txt");
    string line;
    jumlahMatkulTerdaftar = 0;

    if (!file.is_open()) {
        cout << "File matkul.txt tidak ditemukan, akan dibuat baru." << endl;
        return;
    }

    while (getline(file, line) && jumlahMatkulTerdaftar < MAX_MATKUL_DB) {
        if (trim(line) == "") continue;

        vector<string> parts;
        size_t start = 0;
        size_t end = 0;

        while ((end = line.find('|', start)) != string::npos) {
            string part = line.substr(start, end - start);
            parts.push_back(trim(part));
            start = end + 1;
        }

        if (start < line.length()) {
            parts.push_back(trim(line.substr(start)));
        }

        if (parts.size() >= 8) { // Sekarang ada 8 bagian dengan hari dan indexSesi
            databaseMatkul[jumlahMatkulTerdaftar].namaMatkul = parts[0];
            databaseMatkul[jumlahMatkulTerdaftar].jamMulai = parts[1];
            databaseMatkul[jumlahMatkulTerdaftar].jamSelesai = parts[2];
            databaseMatkul[jumlahMatkulTerdaftar].ruangan = parts[3];
            try {
                databaseMatkul[jumlahMatkulTerdaftar].sks = stoi(parts[4]);
                databaseMatkul[jumlahMatkulTerdaftar].indexSesi = stoi(parts[7]);
            } catch (...) {
                databaseMatkul[jumlahMatkulTerdaftar].sks = 3;
                databaseMatkul[jumlahMatkulTerdaftar].indexSesi = -1;
            }
            databaseMatkul[jumlahMatkulTerdaftar].dosen = parts[5];
            databaseMatkul[jumlahMatkulTerdaftar].hari = parts[6];

            jumlahMatkulTerdaftar++;
        }
    }
    file.close();
}

void muatDataMahasiswa() {
    ifstream file("mahasiswa.txt");
    string line;
    jumlahMhsTerdaftar = 0;

    if (!file.is_open()) {
        cout << "File mahasiswa.txt tidak ditemukan, akan dibuat baru." << endl;
        return;
    }

    // Reset database mahasiswa
    for (int i = 0; i < MAX_MAHASISWA; i++) {
        databaseMhs[i].nama = "";
        databaseMhs[i].npm = "";
        for (int j = 1; j <= MAX_SLOTS; j++) {
            databaseMhs[i].jadwal[j] = "Kosong";
            databaseMhs[i].ruangan[j] = "-";
            databaseMhs[i].jamMulai[j] = "-";
            databaseMhs[i].jamSelesai[j] = "-";
            databaseMhs[i].hari[j] = "-";
        }
    }

    while (getline(file, line) && jumlahMhsTerdaftar < MAX_MAHASISWA) {
        if (trim(line) == "") continue;

        vector<string> parts;
        size_t start = 0;
        size_t end = 0;

        for (int i = 0; i < 2; i++) {
            end = line.find('|', start);
            if (end == string::npos) break;
            parts.push_back(trim(line.substr(start, end - start)));
            start = end + 1;
        }

        if (parts.size() < 2) continue;

        Mahasiswa m;
        m.nama = parts[0];
        m.npm = parts[1];

        for (int k = 1; k <= MAX_SLOTS; k++) {
            m.jadwal[k] = "Kosong";
            m.ruangan[k] = "-";
            m.jamMulai[k] = "-";
            m.jamSelesai[k] = "-";
            m.hari[k] = "-";
        }

        string sisa = line.substr(start);
        vector<string> matkulList;
        size_t matkulStart = 0;

        while ((end = sisa.find('|', matkulStart)) != string::npos) {
            string matkul = trim(sisa.substr(matkulStart, end - matkulStart));
            if (!matkul.empty()) {
                matkulList.push_back(matkul);
            }
            matkulStart = end + 1;
        }

        string lastMatkul = trim(sisa.substr(matkulStart));
        if (!lastMatkul.empty()) {
            matkulList.push_back(lastMatkul);
        }

        int slotIndex = 1;
        for (const string& matkulName : matkulList) {
            if (slotIndex > MAX_SLOTS) break;

            MatkulMaster mk;
            if (cariMatkul(matkulName, mk)) {
                m.jadwal[slotIndex] = matkulName;
                m.ruangan[slotIndex] = mk.ruangan;
                m.jamMulai[slotIndex] = mk.jamMulai;
                m.jamSelesai[slotIndex] = mk.jamSelesai;
                m.hari[slotIndex] = mk.hari;
                slotIndex++;
            }
        }

        databaseMhs[jumlahMhsTerdaftar] = m;
        jumlahMhsTerdaftar++;
    }
    file.close();
}

// --- FUNGSI INPUT DATA BARU ---
void tambahMatkulBaru() {
    string namaMK, ruangMK, dosenMK, hariMK;
    int sksMK, pilihanSesi;

    printHeader("TAMBAH MATA KULIAH BARU");

    cout << "\nMasukkan Nama Matkul    : ";
    getline(cin, namaMK);

    // Input SKS dengan validasi
    sksMK = inputInteger("Masukkan SKS (1-3)      : ", 1, 3);

    // Pilih hari
    hariMK = pilihHari();
    cout << "   [OK] Hari terpilih: " << hariMK << endl;

    // Tampilkan pilihan sesi berdasarkan SKS
    tampilkanPilihanSesi(sksMK);

    // Input pilihan sesi dengan validasi
    bool sesiValid = false;
    SesiJam sesiTerpilih;

    do {
        pilihanSesi = inputInteger("Pilih sesi jam (masukkan angka): ", 1, JUMLAH_SESI);

        if (dapatkanSesiBerdasarkanPilihan(sksMK, pilihanSesi, sesiTerpilih)) {
            sesiValid = true;
            cout << "   [OK] Sesi terpilih: " << sesiTerpilih.jamMulai
                 << " - " << sesiTerpilih.jamSelesai << endl;
        } else {
            cout << "   [!] Pilihan sesi tidak valid untuk " << sksMK << " SKS.\n" << endl;
        }
    } while (!sesiValid);

    cout << "Masukkan Ruangan        : ";
    getline(cin, ruangMK);
    cout << "Masukkan Nama Dosen     : ";
    getline(cin, dosenMK);

    // Cari index sesi yang dipilih dalam array SESI_JAM
    int indexSesi = -1;
    for (int i = 0; i < JUMLAH_SESI; i++) {
        if (SESI_JAM[i].jamMulai == sesiTerpilih.jamMulai &&
            SESI_JAM[i].jamSelesai == sesiTerpilih.jamSelesai) {
            indexSesi = i;
            break;
        }
    }

    ofstream file("matkul.txt", ios::app);
    if (file.is_open()) {
        // Format: Nama | JamMulai | JamSelesai | Ruangan | SKS | Dosen | Hari | IndexSesi
        file << namaMK << " | " << sesiTerpilih.jamMulai << " | " << sesiTerpilih.jamSelesai
             << " | " << ruangMK << " | " << sksMK << " | " << dosenMK
             << " | " << hariMK << " | " << indexSesi << endl;

        cout << "\n>> Sukses! Mata kuliah berhasil ditambahkan.\n" << endl;
        file.close();
        muatDataMatkul();
    } else {
        cout << "\n>> Gagal menulis file.\n" << endl;
    }
}

void tambahMahasiswaBaru() {
    string namaMhs, npmMhs, matkulInput, gabunganMatkul = "";

    printHeader("TAMBAH DATA MAHASISWA BARU");
    cout << "\nNama Mahasiswa: ";
    getline(cin, namaMhs);
    cout << "NPM Mahasiswa : ";
    getline(cin, npmMhs);

    // Cari mahasiswa untuk cek bentrok
    Mahasiswa mhs;
    mhs.nama = namaMhs;
    mhs.npm = npmMhs;
    for (int k = 1; k <= MAX_SLOTS; k++) {
        mhs.jadwal[k] = "Kosong";
        mhs.ruangan[k] = "-";
        mhs.jamMulai[k] = "-";
        mhs.jamSelesai[k] = "-";
        mhs.hari[k] = "-";
    }

    cout << "\nMasukkan Matkul yang diambil (Ketik '0' jika selesai):\n" << endl;

    int count = 1;
    int totalSKS = 0;
    while(true) {
        cout << "Matkul ke-" << count << ": ";
        getline(cin, matkulInput);

        if (matkulInput == "0") break;

        MatkulMaster mk;
        if (!cariMatkul(matkulInput, mk)) {
            cout << "   [!] Peringatan: Matkul tidak ditemukan!\n" << endl;
        } else {
            // Cek bentrok jadwal
            if (cekBentrokJadwal(mhs, mk.hari, mk.jamMulai, mk.jamSelesai)) {
                cout << "   [!] PERINGATAN: Matkul ini bentrok dengan jadwal yang sudah dipilih!\n";
                cout << "   [!] Hari: " << mk.hari << ", Jam: " << mk.jamMulai << "-" << mk.jamSelesai << endl;
                cout << "   [!] Silakan pilih matkul lain.\n" << endl;
                continue;
            }
            
            totalSKS += mk.sks;
            cout << "   [OK] " << mk.hari << " | " << mk.jamMulai << "-" << mk.jamSelesai
                 << " | " << mk.sks << " SKS | " << mk.ruangan
                 << " | Dosen: " << mk.dosen << "\n" << endl;
            
            // Update data sementara mahasiswa
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (mhs.jadwal[s] == "Kosong") {
                    mhs.jadwal[s] = matkulInput;
                    mhs.ruangan[s] = mk.ruangan;
                    mhs.jamMulai[s] = mk.jamMulai;
                    mhs.jamSelesai[s] = mk.jamSelesai;
                    mhs.hari[s] = mk.hari;
                    break;
                }
            }
        }

        if (gabunganMatkul != "") gabunganMatkul += "|";
        gabunganMatkul += matkulInput;
        count++;
        
        // Cek apakah sudah mencapai maksimum slot
        if (count > MAX_SLOTS + 1) {
            cout << "   [!] Maksimum " << MAX_SLOTS << " matkul telah dipilih.\n" << endl;
            break;
        }
    }

    cout << "\nTotal SKS yang diambil: " << totalSKS << " SKS\n" << endl;

    ofstream file("mahasiswa.txt", ios::app);
    if (file.is_open()) {
        file << namaMhs << " | " << npmMhs << " | " << gabunganMatkul << endl;
        cout << ">> Sukses! Data mahasiswa berhasil ditambahkan.\n" << endl;
        file.close();
        muatDataMahasiswa();
    } else {
        cout << ">> Gagal menulis file.\n" << endl;
    }
}

// --- FUNGSI EDIT DATA ---
void editMatkul() {
    if (jumlahMatkulTerdaftar == 0) {
        cout << "\n>> Database matkul masih kosong.\n" << endl;
        return;
    }

    printHeader("EDIT MATA KULIAH");

    cout << "\nDaftar Mata Kuliah:" << endl;
    printLine();
    for (int i = 0; i < jumlahMatkulTerdaftar; i++) {
        cout << (i+1) << ". " << databaseMatkul[i].namaMatkul
             << " (" << databaseMatkul[i].hari << " "
             << databaseMatkul[i].jamMulai << "-"
             << databaseMatkul[i].jamSelesai << ")" << endl;
    }
    printLine();

    int pilih = inputInteger("\nPilih nomor matkul yang akan diedit (0 untuk batal): ", 0, jumlahMatkulTerdaftar);

    if (pilih == 0) {
        cout << "\n>> Batal edit.\n" << endl;
        return;
    }

    int idx = pilih - 1;
    string namaBaru, ruanganBaru, dosenBaru, hariBaru;
    int sksBaru, pilihanSesiBaru;

    // Simpan data lama untuk update
    string namaLama = databaseMatkul[idx].namaMatkul;
    string hariLama = databaseMatkul[idx].hari;
    string jamMulaiLama = databaseMatkul[idx].jamMulai;
    string jamSelesaiLama = databaseMatkul[idx].jamSelesai;

    cout << "\nData saat ini:" << endl;
    cout << "Nama   : " << databaseMatkul[idx].namaMatkul << endl;
    cout << "Hari   : " << databaseMatkul[idx].hari << endl;
    cout << "Waktu  : " << databaseMatkul[idx].jamMulai << "-" << databaseMatkul[idx].jamSelesai << endl;
    cout << "Ruangan: " << databaseMatkul[idx].ruangan << endl;
    cout << "SKS    : " << databaseMatkul[idx].sks << endl;
    cout << "Dosen  : " << databaseMatkul[idx].dosen << endl;

    cout << "\n(Tekan Enter untuk tetap menggunakan nilai lama)\n" << endl;

    cout << "Nama Baru     : ";
    getline(cin, namaBaru);
    if (namaBaru != "") databaseMatkul[idx].namaMatkul = namaBaru;

    // Input hari baru
    cout << "Hari Baru (ketik 'ganti' untuk mengganti, atau Enter untuk tetap): ";
    string hariInput;
    getline(cin, hariInput);
    if (hariInput == "ganti") {
        hariBaru = pilihHari();
        databaseMatkul[idx].hari = hariBaru;
        cout << "   [OK] Hari terpilih: " << hariBaru << endl;
    }

    // Jika ingin mengubah SKS, tampilkan sesi baru
    cout << "SKS Baru (masukkan 0 jika tidak ingin mengubah): ";
    string sksInput;
    getline(cin, sksInput);

    if (sksInput != "" && sksInput != "0") {
        try {
            sksBaru = stoi(sksInput);
            if (sksBaru >= 1 && sksBaru <= 3) {
                databaseMatkul[idx].sks = sksBaru;

                // Tampilkan pilihan sesi baru berdasarkan SKS baru
                tampilkanPilihanSesi(sksBaru);
                pilihanSesiBaru = inputInteger("Pilih sesi jam baru: ", 1, JUMLAH_SESI);

                SesiJam sesiBaru;
                if (dapatkanSesiBerdasarkanPilihan(sksBaru, pilihanSesiBaru, sesiBaru)) {
                    databaseMatkul[idx].jamMulai = sesiBaru.jamMulai;
                    databaseMatkul[idx].jamSelesai = sesiBaru.jamSelesai;

                    // Update index sesi
                    for (int i = 0; i < JUMLAH_SESI; i++) {
                        if (SESI_JAM[i].jamMulai == sesiBaru.jamMulai &&
                            SESI_JAM[i].jamSelesai == sesiBaru.jamSelesai) {
                            databaseMatkul[idx].indexSesi = i;
                            break;
                        }
                    }
                }
            }
        } catch (...) {
            cout << "   [!] Input SKS tidak valid, menggunakan nilai lama.\n" << endl;
        }
    }

    cout << "Ruangan Baru  : ";
    getline(cin, ruanganBaru);
    if (ruanganBaru != "") databaseMatkul[idx].ruangan = ruanganBaru;

    cout << "Dosen Baru    : ";
    getline(cin, dosenBaru);
    if (dosenBaru != "") databaseMatkul[idx].dosen = dosenBaru;

    // Cek bentrok untuk mahasiswa yang mengambil matkul ini
    bool adaBentrok = false;
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (databaseMhs[i].jadwal[s] == namaLama) {
                Mahasiswa tempMhs = databaseMhs[i];
                // Ganti data matkul lama dengan yang baru di tempMhs
                for (int ts = 1; ts <= MAX_SLOTS; ts++) {
                    if (tempMhs.jadwal[ts] == namaLama) {
                        tempMhs.hari[ts] = databaseMatkul[idx].hari;
                        tempMhs.jamMulai[ts] = databaseMatkul[idx].jamMulai;
                        tempMhs.jamSelesai[ts] = databaseMatkul[idx].jamSelesai;
                    }
                }
                
                // Cek bentrok dengan matkul lain
                if (cekBentrokJadwal(tempMhs, databaseMatkul[idx].hari, 
                                     databaseMatkul[idx].jamMulai, 
                                     databaseMatkul[idx].jamSelesai, 
                                     namaLama)) {
                    adaBentrok = true;
                    cout << "\n   [!] PERINGATAN: Perubahan ini akan menyebabkan bentrok jadwal untuk mahasiswa:\n";
                    cout << "   [!] " << databaseMhs[i].nama << " (" << databaseMhs[i].npm << ")\n";
                    cout << "   [!] Matkul: " << namaLama << endl;
                }
                break;
            }
        }
    }

    if (adaBentrok) {
        cout << "\nApakah Anda tetap ingin menyimpan perubahan? (y/n): ";
        char konfirmasi;
        cin >> konfirmasi;
        cin.ignore();
        
        if (konfirmasi != 'y' && konfirmasi != 'Y') {
            cout << "\n>> Perubahan dibatalkan.\n" << endl;
            // Kembalikan nilai lama
            databaseMatkul[idx].namaMatkul = namaLama;
            databaseMatkul[idx].hari = hariLama;
            databaseMatkul[idx].jamMulai = jamMulaiLama;
            databaseMatkul[idx].jamSelesai = jamSelesaiLama;
            return;
        }
    }

    // Tulis ulang file matkul.txt
    ofstream file("matkul.txt");
    if (file.is_open()) {
        for (int i = 0; i < jumlahMatkulTerdaftar; i++) {
            file << databaseMatkul[i].namaMatkul << " | "
                 << databaseMatkul[i].jamMulai << " | "
                 << databaseMatkul[i].jamSelesai << " | "
                 << databaseMatkul[i].ruangan << " | "
                 << databaseMatkul[i].sks << " | "
                 << databaseMatkul[i].dosen << " | "
                 << databaseMatkul[i].hari << " | "
                 << databaseMatkul[i].indexSesi << endl;
        }
        file.close();

        // UPDATE PENTING: Update juga data di file mahasiswa.txt jika ada perubahan
        if (namaBaru != "" && namaBaru != namaLama) {
            // Update di database array terlebih dahulu
            for (int i = 0; i < jumlahMhsTerdaftar; i++) {
                for (int s = 1; s <= MAX_SLOTS; s++) {
                    if (databaseMhs[i].jadwal[s] == namaLama) {
                        databaseMhs[i].jadwal[s] = namaBaru;
                    }
                }
            }
        }

        // Update hari, jam, dan ruangan di jadwal mahasiswa
        for (int i = 0; i < jumlahMhsTerdaftar; i++) {
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (databaseMhs[i].jadwal[s] == databaseMatkul[idx].namaMatkul) {
                    databaseMhs[i].ruangan[s] = databaseMatkul[idx].ruangan;
                    databaseMhs[i].jamMulai[s] = databaseMatkul[idx].jamMulai;
                    databaseMhs[i].jamSelesai[s] = databaseMatkul[idx].jamSelesai;
                    databaseMhs[i].hari[s] = databaseMatkul[idx].hari;
                }
            }
        }

        // Tulis ulang file mahasiswa.txt dengan data yang sudah diupdate
        ofstream fileMhs("mahasiswa.txt");
        if (fileMhs.is_open()) {
            for (int i = 0; i < jumlahMhsTerdaftar; i++) {
                fileMhs << databaseMhs[i].nama << " | " << databaseMhs[i].npm << " | ";

                bool first = true;
                for (int s = 1; s <= MAX_SLOTS; s++) {
                    if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                        if (!first) fileMhs << "|";
                        fileMhs << databaseMhs[i].jadwal[s];
                        first = false;
                    }
                }
                fileMhs << endl;
            }
            fileMhs.close();
        }

        cout << "\n>> Data berhasil diupdate!\n" << endl;

        // Muat ulang data
        muatDataMatkul();
        muatDataMahasiswa();

    } else {
        cout << "\n>> Gagal menulis file.\n" << endl;
    }
}

void editMahasiswa() {
    if (jumlahMhsTerdaftar == 0) {
        cout << "\n>> Database mahasiswa masih kosong.\n" << endl;
        return;
    }

    printHeader("EDIT DATA MAHASISWA");

    cout << "\nDaftar Mahasiswa:" << endl;
    printLine();
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        cout << (i+1) << ". " << databaseMhs[i].nama
             << " (" << databaseMhs[i].npm << ")" << endl;
    }
    printLine();

    int pilih = inputInteger("\nPilih nomor mahasiswa yang akan diedit (0 untuk batal): ", 0, jumlahMhsTerdaftar);

    if (pilih == 0) {
        cout << "\n>> Batal edit.\n" << endl;
        return;
    }

    int idx = pilih - 1;
    string namaBaru, npmBaru;

    cout << "\nData saat ini:" << endl;
    cout << "Nama: " << databaseMhs[idx].nama << endl;
    cout << "NPM : " << databaseMhs[idx].npm << endl;

    cout << "\n(Tekan Enter untuk tetap menggunakan nilai lama)\n" << endl;

    cout << "Nama Baru: ";
    getline(cin, namaBaru);
    if (namaBaru != "") databaseMhs[idx].nama = namaBaru;

    cout << "NPM Baru : ";
    getline(cin, npmBaru);
    if (npmBaru != "") databaseMhs[idx].npm = npmBaru;

    // Tulis ulang file
    ofstream file("mahasiswa.txt");
    if (file.is_open()) {
        for (int i = 0; i < jumlahMhsTerdaftar; i++) {
            file << databaseMhs[i].nama << " | " << databaseMhs[i].npm << " | ";

            bool first = true;
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                    if (!first) file << "|";
                    file << databaseMhs[i].jadwal[s];
                    first = false;
                }
            }
            file << endl;
        }
        file.close();
        cout << "\n>> Data berhasil diupdate!\n" << endl;
    } else {
        cout << "\n>> Gagal menulis file.\n" << endl;
    }
}

// --- FUNGSI HAPUS DATA ---
void hapusMatkul() {
    if (jumlahMatkulTerdaftar == 0) {
        cout << "\n>> Database matkul masih kosong.\n" << endl;
        return;
    }

    printHeader("HAPUS MATA KULIAH");

    cout << "\nDaftar Mata Kuliah:" << endl;
    printLine();
    for (int i = 0; i < jumlahMatkulTerdaftar; i++) {
        cout << (i+1) << ". " << databaseMatkul[i].namaMatkul
             << " (" << databaseMatkul[i].hari << " "
             << databaseMatkul[i].jamMulai << "-"
             << databaseMatkul[i].jamSelesai << ")" << endl;
    }
    printLine();

    int pilih = inputInteger("\nPilih nomor matkul yang akan dihapus (0 untuk batal): ", 0, jumlahMatkulTerdaftar);

    if (pilih == 0) {
        cout << "\n>> Batal hapus.\n" << endl;
        return;
    }

    int idx = pilih - 1;
    string namaMatkul = databaseMatkul[idx].namaMatkul;

    cout << "\nApakah Anda yakin ingin menghapus mata kuliah '" << namaMatkul << "'? (y/n): ";
    char konfirmasi;
    cin >> konfirmasi;
    cin.ignore();

    if (konfirmasi != 'y' && konfirmasi != 'Y') {
        cout << "\n>> Batal hapus.\n" << endl;
        return;
    }

    // Hapus dari array (geser elemen)
    for (int i = idx; i < jumlahMatkulTerdaftar - 1; i++) {
        databaseMatkul[i] = databaseMatkul[i + 1];
    }
    jumlahMatkulTerdaftar--;

    // Tulis ulang file
    ofstream file("matkul.txt");
    if (file.is_open()) {
        for (int i = 0; i < jumlahMatkulTerdaftar; i++) {
            file << databaseMatkul[i].namaMatkul << " | "
                 << databaseMatkul[i].jamMulai << " | "
                 << databaseMatkul[i].jamSelesai << " | "
                 << databaseMatkul[i].ruangan << " | "
                 << databaseMatkul[i].sks << " | "
                 << databaseMatkul[i].dosen << " | "
                 << databaseMatkul[i].hari << " | "
                 << databaseMatkul[i].indexSesi << endl;
        }
        file.close();

        // Hapus matkul dari jadwal mahasiswa yang mengambilnya
        for (int i = 0; i < jumlahMhsTerdaftar; i++) {
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (databaseMhs[i].jadwal[s] == namaMatkul) {
                    databaseMhs[i].jadwal[s] = "Kosong";
                    databaseMhs[i].ruangan[s] = "-";
                    databaseMhs[i].jamMulai[s] = "-";
                    databaseMhs[i].jamSelesai[s] = "-";
                    databaseMhs[i].hari[s] = "-";
                }
            }
        }

        // Update file mahasiswa
        ofstream fileMhs("mahasiswa.txt");
        if (fileMhs.is_open()) {
            for (int i = 0; i < jumlahMhsTerdaftar; i++) {
                fileMhs << databaseMhs[i].nama << " | " << databaseMhs[i].npm << " | ";

                bool first = true;
                for (int s = 1; s <= MAX_SLOTS; s++) {
                    if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                        if (!first) fileMhs << "|";
                        fileMhs << databaseMhs[i].jadwal[s];
                        first = false;
                    }
                }
                fileMhs << endl;
            }
            fileMhs.close();
        }

        cout << "\n>> Mata kuliah '" << namaMatkul << "' berhasil dihapus!\n" << endl;
    } else {
        cout << "\n>> Gagal menulis file.\n" << endl;
    }
}

void hapusMahasiswa() {
    if (jumlahMhsTerdaftar == 0) {
        cout << "\n>> Database mahasiswa masih kosong.\n" << endl;
        return;
    }

    printHeader("HAPUS DATA MAHASISWA");

    cout << "\nDaftar Mahasiswa:" << endl;
    printLine();
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        cout << (i+1) << ". " << databaseMhs[i].nama
             << " (" << databaseMhs[i].npm << ")" << endl;
    }
    printLine();

    int pilih = inputInteger("\nPilih nomor mahasiswa yang akan dihapus (0 untuk batal): ", 0, jumlahMhsTerdaftar);

    if (pilih == 0) {
        cout << "\n>> Batal hapus.\n" << endl;
        return;
    }

    int idx = pilih - 1;
    string namaMhs = databaseMhs[idx].nama;

    cout << "\nApakah Anda yakin ingin menghapus mahasiswa '" << namaMhs << "'? (y/n): ";
    char konfirmasi;
    cin >> konfirmasi;
    cin.ignore();

    if (konfirmasi != 'y' && konfirmasi != 'Y') {
        cout << "\n>> Batal hapus.\n" << endl;
        return;
    }

    // Hapus dari array (geser elemen)
    for (int i = idx; i < jumlahMhsTerdaftar - 1; i++) {
        databaseMhs[i] = databaseMhs[i + 1];
    }
    jumlahMhsTerdaftar--;

    // Tulis ulang file
    ofstream file("mahasiswa.txt");
    if (file.is_open()) {
        for (int i = 0; i < jumlahMhsTerdaftar; i++) {
            file << databaseMhs[i].nama << " | " << databaseMhs[i].npm << " | ";

            bool first = true;
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                    if (!first) file << "|";
                    file << databaseMhs[i].jadwal[s];
                    first = false;
                }
            }
            file << endl;
        }
        file.close();
        cout << "\n>> Mahasiswa '" << namaMhs << "' berhasil dihapus!\n" << endl;
    } else {
        cout << "\n>> Gagal menulis file.\n" << endl;
    }
}

// --- OUTPUT ---
void tampilkanSemuaMatkul() {
    printHeader("DATABASE MATA KULIAH");

    if (jumlahMatkulTerdaftar == 0) {
        cout << "\n(Database masih kosong)\n" << endl;
        return;
    }

    cout << "\n" << left << setw(4) << "No"
         << setw(30) << "Mata Kuliah"
         << setw(8) << "SKS"
         << setw(12) << "Hari"
         << setw(20) << "Waktu"
         << setw(10) << "Ruangan"
         << "Dosen" << endl;
    printLine();

    for (int i = 0; i < jumlahMatkulTerdaftar; i++) {
        string waktu = databaseMatkul[i].jamMulai + "-" + databaseMatkul[i].jamSelesai;
        cout << left << setw(4) << (i + 1)
             << setw(30) << databaseMatkul[i].namaMatkul
             << setw(8) << databaseMatkul[i].sks
             << setw(12) << databaseMatkul[i].hari
             << setw(20) << waktu
             << setw(10) << databaseMatkul[i].ruangan
             << databaseMatkul[i].dosen << endl;
    }
    printLine();
    cout << "\nTotal: " << jumlahMatkulTerdaftar << " mata kuliah terdaftar.\n" << endl;
}

void tampilkanJadwal(string namaDicari) {
    bool ditemukan = false;
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        if (databaseMhs[i].nama == namaDicari) {
            ditemukan = true;

            int panjangHeader = max(70, (int)namaDicari.length() + 30);
            cout << "\n" << string(panjangHeader, '=') << endl;
            cout << "  JADWAL PERKULIAHAN" << endl;
            cout << string(panjangHeader, '=') << endl;
            cout << "\nNama : " << databaseMhs[i].nama << endl;
            cout << "NPM  : " << databaseMhs[i].npm << "\n" << endl;

            cout << left << setw(6) << "No"
                 << setw(12) << "Hari"
                 << setw(20) << "Waktu"
                 << setw(30) << "Mata Kuliah"
                 << setw(10) << "Ruangan"
                 << "Status" << endl;
            printLineDynamic(88);

            int totalKelas = 0, totalSKS = 0;
            int no = 1;
            
            // Urutkan berdasarkan hari dan jam
            vector<pair<int, string>> jadwalUrut; // index slot, keterangan
            
            for (int s = 1; s <= MAX_SLOTS; s++) {
                if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                    string keterangan = databaseMhs[i].hari[s] + "|" + 
                                       databaseMhs[i].jamMulai[s] + "|" + 
                                       to_string(s);
                    jadwalUrut.push_back({s, keterangan});
                }
            }
            
            // Urutkan berdasarkan hari kemudian jam
            sort(jadwalUrut.begin(), jadwalUrut.end(), [](const pair<int, string>& a, const pair<int, string>& b) {
                // Urutkan berdasarkan hari
                string hariA = a.second.substr(0, a.second.find('|'));
                string hariB = b.second.substr(0, b.second.find('|'));
                
                // Cari indeks hari
                int idxA = -1, idxB = -1;
                for (int h = 0; h < JUMLAH_HARI; h++) {
                    if (HARI[h] == hariA) idxA = h;
                    if (HARI[h] == hariB) idxB = h;
                }
                
                if (idxA != idxB) return idxA < idxB;
                
                // Jika hari sama, urutkan berdasarkan jam
                string jamA = a.second.substr(hariA.length() + 1);
                jamA = jamA.substr(0, jamA.find('|'));
                string jamB = b.second.substr(hariB.length() + 1);
                jamB = jamB.substr(0, jamB.find('|'));
                
                return jamA < jamB;
            });
            
            for (const auto& item : jadwalUrut) {
                int s = item.first;
                totalKelas++;
                string waktu = databaseMhs[i].jamMulai[s] + "-" + databaseMhs[i].jamSelesai[s];

                MatkulMaster mk;
                if (cariMatkul(databaseMhs[i].jadwal[s], mk)) {
                    totalSKS += mk.sks;
                }

                cout << left << setw(6) << no++
                     << setw(12) << databaseMhs[i].hari[s]
                     << setw(20) << waktu
                     << setw(30) << databaseMhs[i].jadwal[s]
                     << setw(10) << databaseMhs[i].ruangan[s]
                     << "Sibuk" << endl;
            }

            if (totalKelas == 0) {
                cout << "        (Tidak ada jadwal yang terdaftar)" << endl;
            }

            printLineDynamic(88);
            cout << "\nTotal Kelas: " << totalKelas << " | Total SKS: " << totalSKS << "\n" << endl;
            break;
        }
    }
    if (!ditemukan) {
        cout << "\n>> Mahasiswa tidak ditemukan dalam database.\n" << endl;
    }
}

void cekKetersediaan(string hari, string waktu) {
    int panjangHeader = max(60, (int)(hari + " " + waktu).length() + 40);
    cout << "\n" << string(panjangHeader, '=') << endl;
    cout << "  CEK KETERSEDIAAN JAM" << endl;
    cout << string(panjangHeader, '=') << endl;
    cout << "\nHari : " << hari << endl;
    cout << "Waktu: " << waktu << "\n" << endl;

    int sibuk = 0, kosong = 0;

    cout << "TIDAK TERSEDIA (Sedang Kuliah):" << endl;
    printLine();
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (databaseMhs[i].hari[s] == hari && databaseMhs[i].jamMulai[s] == waktu) {
                sibuk++;
                cout << sibuk << ". " << databaseMhs[i].nama
                     << " (" << databaseMhs[i].npm << ")" << endl;
                cout << "   Matkul : " << databaseMhs[i].jadwal[s]
                     << " | Ruangan: " << databaseMhs[i].ruangan[s] << endl;
                break;
            }
        }
    }
    if (sibuk == 0) cout << "(Tidak ada mahasiswa yang sibuk)\n" << endl;

    cout << "\nTERSEDIA (Jam Kosong):" << endl;
    printLine();
    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        bool tersedia = true;
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (databaseMhs[i].hari[s] == hari && databaseMhs[i].jamMulai[s] == waktu) {
                tersedia = false;
                break;
            }
        }
        if (tersedia) {
            kosong++;
            cout << kosong << ". " << databaseMhs[i].nama
                 << " (" << databaseMhs[i].npm << ")" << endl;
        }
    }
    if (kosong == 0) cout << "(Semua mahasiswa sedang sibuk)\n" << endl;

    printLine();
    cout << "\nRingkasan: " << sibuk << " Sibuk | " << kosong << " Tersedia\n" << endl;
}

void tampilkanSemuaJadwal() {
    if (jumlahMhsTerdaftar == 0) {
        cout << "\n(Belum ada data mahasiswa)\n" << endl;
        return;
    }

    printHeader("DAFTAR SEMUA MAHASISWA");
    cout << "\n" << left << setw(4) << "No"
         << setw(25) << "Nama"
         << setw(15) << "NPM"
         << "Total Kelas" << endl;
    printLine();

    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        int totalKelas = 0;
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                totalKelas++;
            }
        }

        cout << left << setw(4) << (i+1)
             << setw(25) << databaseMhs[i].nama
             << setw(15) << databaseMhs[i].npm
             << totalKelas << " kelas" << endl;
    }
    printLine();
    cout << "\nTotal: " << jumlahMhsTerdaftar << " mahasiswa terdaftar.\n" << endl;
}

void exportJadwal() {
    if (jumlahMhsTerdaftar == 0) {
        cout << "\n>> Tidak ada data untuk di-export.\n" << endl;
        return;
    }

    ofstream file("export_jadwal.txt");
    if (!file.is_open()) {
        cout << "\n>> Gagal membuat file export.\n" << endl;
        return;
    }

    file << "============================================================\n";
    file << "         LAPORAN JADWAL PERKULIAHAN MAHASISWA\n";
    file << "============================================================\n\n";

    for (int i = 0; i < jumlahMhsTerdaftar; i++) {
        file << "Mahasiswa: " << databaseMhs[i].nama
             << " (" << databaseMhs[i].npm << ")\n";
        file << "------------------------------------------------------------\n";

        // Urutkan berdasarkan hari dan jam
        vector<pair<int, string>> jadwalUrut; // index slot, keterangan
        
        for (int s = 1; s <= MAX_SLOTS; s++) {
            if (databaseMhs[i].jadwal[s] != "Kosong" && databaseMhs[i].jadwal[s] != "-") {
                string keterangan = databaseMhs[i].hari[s] + "|" + 
                                   databaseMhs[i].jamMulai[s] + "|" + 
                                   to_string(s);
                jadwalUrut.push_back({s, keterangan});
            }
        }
        
        // Urutkan berdasarkan hari kemudian jam
        sort(jadwalUrut.begin(), jadwalUrut.end(), [](const pair<int, string>& a, const pair<int, string>& b) {
            string hariA = a.second.substr(0, a.second.find('|'));
            string hariB = b.second.substr(0, b.second.find('|'));
            
            int idxA = -1, idxB = -1;
            for (int h = 0; h < JUMLAH_HARI; h++) {
                if (HARI[h] == hariA) idxA = h;
                if (HARI[h] == hariB) idxB = h;
            }
            
            if (idxA != idxB) return idxA < idxB;
            
            string jamA = a.second.substr(hariA.length() + 1);
            jamA = jamA.substr(0, jamA.find('|'));
            string jamB = b.second.substr(hariB.length() + 1);
            jamB = jamB.substr(0, jamB.find('|'));
            
            return jamA < jamB;
        });
        
        int totalKelas = 0;
        for (const auto& item : jadwalUrut) {
            int s = item.first;
            totalKelas++;
            string waktu = databaseMhs[i].jamMulai[s] + "-" + databaseMhs[i].jamSelesai[s];
            file << "Hari : " << databaseMhs[i].hari[s] << "\n";
            file << "Waktu: " << waktu << "\n";
            file << "  Matkul : " << databaseMhs[i].jadwal[s] << "\n";
            file << "  Ruangan: " << databaseMhs[i].ruangan[s] << "\n";

            MatkulMaster mk;
            if (cariMatkul(databaseMhs[i].jadwal[s], mk)) {
                file << "  Dosen  : " << mk.dosen << "\n";
                file << "  SKS    : " << mk.sks << "\n\n";
            }
        }
        file << "Total: " << totalKelas << " kelas\n";
        file << "============================================================\n\n";
    }

    file.close();
    cout << "\n>> Jadwal berhasil di-export ke 'export_jadwal.txt'\n" << endl;
}

// Fungsi untuk clear terminal
void clearTerminal() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    // Buat file kosong jika belum ada
    ofstream f1("matkul.txt", ios::app); f1.close();
    ofstream f2("mahasiswa.txt", ios::app); f2.close();

    // Muat data dari file
    muatDataMatkul();
    muatDataMahasiswa();

    int pilihan;
    do {
        printHeader("SISTEM PENJADWALAN PERKULIAHAN");
        cout << "\nMENU UTAMA:\n" << endl;
        cout << "  1. Input Data Baru" << endl;
        cout << "  2. Lihat Jadwal Mahasiswa (Per Orang)" << endl;
        cout << "  3. Lihat Semua Mahasiswa Terdaftar" << endl;
        cout << "  4. Cek Ketersediaan Jam (Siapa yang kosong)" << endl;
        cout << "  5. Edit Data" << endl;
        cout << "  6. Hapus Data" << endl;
        cout << "  7. Lihat Database Mata Kuliah" << endl;
        cout << "  8. Export Jadwal ke File" << endl;
        cout << "  9. Clear Terminal" << endl;
        cout << "  0. Keluar" << endl;

        cout << "\n=== Status Database Saat Ini ===" << endl;
        cout << "Data berhasil dimuat:" << endl;
        cout << "- Mata kuliah terdaftar: " << jumlahMatkulTerdaftar << endl;
        cout << "- Mahasiswa terdaftar: " << jumlahMhsTerdaftar << endl;

        cout << "\n------------------------------------------------------------" << endl;
        cout << "Pilihan Anda: ";

        // Validasi input menu utama
        while (!(cin >> pilihan)) {
            cout << "\n   [!] Input harus berupa angka! Silakan coba lagi.\n" << endl;
            cout << "Pilihan Anda: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore();

        switch(pilihan) {
            case 1: {
                char subPilih;
                do{
                cout << "\n   a. Tambah Mata Kuliah" << endl;
                cout << "   b. Tambah Data Mahasiswa" << endl;
                cout << "\n   Pilihan (a/b) (ketik 0 jika selesai): ";
                cin >> subPilih;
                cin.ignore();

                if (subPilih == 'a' || subPilih == 'A') {
                    tambahMatkulBaru();
                } else if (subPilih == 'b' || subPilih == 'B') {
                    tampilkanSemuaMatkul();
                    tambahMahasiswaBaru();}
                else if (subPilih =='0'){
                    break;
                }
                else{
                    cout << "   Input berupa huruf a/b!!" <<endl;
                }
                }while(subPilih != 'a' || subPilih != 'A' || subPilih != 'b' || subPilih != 'B');
                break;
            }
            case 2: {
                string nama;
                cout << "\nMasukkan Nama Mahasiswa: ";
                getline(cin, nama);
                tampilkanJadwal(nama);
                break;
            }
            case 3: {
                tampilkanSemuaJadwal();
                break;
            }
            case 4: {
                string hari, waktu;
                
                // Pilih hari
                tampilkanPilihanHari();
                int pilihanHari = inputInteger("Pilih hari (1-5): ", 1, JUMLAH_HARI);
                hari = HARI[pilihanHari - 1];
                
                cout << "\nMasukkan Jam Mulai (HH.MM, contoh: 07.30): ";
                getline(cin, waktu);

                // Validasi format waktu
                if (waktu.length() == 5 && waktu[2] == '.') {
                    cekKetersediaan(hari, waktu);
                } else {
                    cout << "\n>> Format waktu tidak valid!\n" << endl;
                }
                break;
            }
            case 5: {
                cout << "\n   a. Edit Mata Kuliah" << endl;
                cout << "   b. Edit Data Mahasiswa" << endl;
                cout << "\n   Pilihan (a/b): ";
                char subPilih;
                cin >> subPilih;
                cin.ignore();

                if (subPilih == 'a' || subPilih == 'A') {
                    editMatkul();
                } else if (subPilih == 'b' || subPilih == 'B') {
                    editMahasiswa();
                }
                break;
            }
            case 6: {
                cout << "\n   a. Hapus Mata Kuliah" << endl;
                cout << "   b. Hapus Data Mahasiswa" << endl;
                cout << "\n   Pilihan (a/b): ";
                char subPilih;
                cin >> subPilih;
                cin.ignore();

                if (subPilih == 'a' || subPilih == 'A') {
                    hapusMatkul();
                } else if (subPilih == 'b' || subPilih == 'B') {
                    hapusMahasiswa();
                }
                break;
            }
            case 7: {
                tampilkanSemuaMatkul();
                break;
            }
            case 8: {
                exportJadwal();
                break;
            }
            case 9: {
                clearTerminal();
                cout << "Terminal telah dibersihkan.\n" << endl;
                continue;
            }
            case 0: {
                printHeader("TERIMA KASIH");
                cout << "\nProgram selesai. Terima kasih banyak telah menggunakan program ini :)\n" << endl;
                break;
            }
            default: {
                cout << "\n>> Pilihan tidak valid. Silakan coba lagi..\n" << endl;
            }
        }

        if (pilihan != 0 && pilihan != 10) {
            cout << "\nTekan Enter untuk melanjutkan...";
            cin.get();
        }

    } while (pilihan != 0);

    return 0;
}