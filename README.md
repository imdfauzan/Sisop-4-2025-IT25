# Laporan Resmi Praktikum SISOP Modul 4
## 👥 Anggota Kelompok

| No | Nama                     | NRP         |
|----|--------------------------|-------------|
| 1  | Tiara Putri Prasetya     | 5027241013  |
| 2  | Danuja Prasasta Bastu    | 5027241037  |
| 3  | Imam Mahmud Dalil Fauzan | 5027241100  |

# SOAL 1
## ✏️ Pembuat: Danuja Prasasta Bastu (5027241037)

## 📂 Ringkasan
Program ini otomatisi dalam proses dari download file ZIP berisi teks kode-kode heksadesimal yang merepresentasikan data gambar, lalu mengkonversi data itu jadi gambar asli (.png) dan disimpan hasilnya di folder khusus. jika ada data gambar tapi masih dalam bentuk tulisan (kode hex), nah program ini yang bakal merubah tulisan itu jadi file gambar yang bisa dibuka kayak biasa.

## ⚙️ Saat di-Run
Program ini akan:
- Download file ZIP dari URL yang  dikasih.
- meng-Unzip file itu, isinya file teks (.txt) yang ada di folder anomali/.
- Cek semua file .txt di folder itu.
- Baca tiap file .txt, yang isinya adalah data gambar dalam bentuk hex string.
- Ubah hex string jadi data biner (yang bisa dimengerti oleh format gambar asli).
- Simpan hasilnya sebagai file gambar .png di folder image/.
- Tulis catatan (log) ke file conversion.log buat ngelacak file mana aja yang udah dikonversi.

## 🧠 Beberapa bagian penting kode ini:
- `create_dir_if_not_exists()` Cek apakah folder (misalnya image/) sudah ada atau belum. Kalau belum ada, bikin foldernya.
- `get_timestamp() dan get_log_timestamp()` mengambil waktu saat ini dan dijadikan string, buat nama file gambar & log yang unik (biar gak bentrok).
- `download_file()` Pake libcurl buat unduh file dari internet. File yang diunduh disimpan sebagai download.zip.
- `unzip_file()` Unzip file yang udah didownload tadi. Pake perintah unzip yang langsung dijalankan lewat `system()`.
- `hex_to_bin()` mengubah string heksadesimal jadi array data biner (yang bisa disimpan jadi gambar).
- `convert_hex_to_image()` Gabungan dari `hex_to_bin()` dan penyimpanan ke file gambar. Jadi intinya: baca hex, ubah ke biner, simpan jadi .png.
- `log_conversion()` Catat konversi dari file .txt ke .png ke dalam file conversion.log.
- `process_hex_files_in_dir()` Bagian ini yang jalanin semua proses konversi untuk tiap file .txt dalam folder anomali/.
- `main()` — Titik Awal Program
Fungsi `main()` menjalankan semua langkah utama:
Cek apakah URL udah dikasih sebagai argumen.
Download file ZIP dari URL tersebut.
Unzip file ZIP-nya.
Hapus ZIP setelah berhasil di-unzip.
Proses semua file .txt yang ada di folder anomali/.
Hasil gambar disimpan di folder image/.
Semua konversi dicatat di log.

## Cara Run Program Ini
Misal diberikan URL ZIP: https://example.com/data.zip, jalankan seperti ini:
```
./converter https://example.com/data.zip
```
Setelah dijalankan, Folder image/ bakal berisi gambar-gambar hasil konversi.
File `conversion.log` bakal berisi catatan:
```
[2025-05-17][10:15:22]: Successfully converted hexadecimal text image1.txt to image1_image_2025-05-17_10:15:22.png.
```

# SOAL 2
## ✏️ Pembuat: Imam Mahmud Dalil Fauzan (5027241100)

## 📂 Baymax - FUSE Filesystem untuk File Virtual Berbasis Fragmen

## 📌 Deskripsi Program
Baymax adalah penerapan filesystem berbasis FUSE yang memvirtualisasikan file Baymax.jpeg pecahan-pecahan file (fragmen) yang disimpan di direktori /relics. File Baymax.jpeg cuma muncul saat filesystem di-mount, dan seluruh operasi baca/tulis ke file ini akan dibagi menjadi fragmen-fragmen kecil berukuran 1024 byte.
Program ini juga mencatat seluruh aktivitas READ, WRITE, dan DELETE file ke dalam log file activity.log.

## ⚙️ Struktur Direktori
```
├── baymax.c         # File program utama
├── relics/             # Folder berisi potongan-potongan fragmen file
│   └── Baymax.jpeg.000
│   └── Baymax.jpeg.001
│   └── ............013
├── activity.log        # File log aktivitas
```

## 🧠 Fitur Utama
- Menyediakan file virtual Baymax.jpeg yang sebenarnya disusun dari banyak fragmen.
- Operasi baca (read) menggabungkan data dari setiap fragmen.
- Operasi tulis (write) memecah data dan menyimpannya ke fragmen-fragmen.
- Operasi hapus (unlink) menghapus semua fragmen terkait file.
- Setiap operasi dicatat ke dalam file activity.log.

## 🛠️ Cara Compile & Run
1. Pastikan FUSE sudah terinstall:
```
sudo apt update
sudo apt install libfuse3-dev build-essential
```
2. Compile program:
```
gcc `pkg-config fuse3 --cflags --libs` baymax_fs.c -o baymax_fs
```
4. Buat direktori mount point (mount_dir) dan run:
```
mkdir mountdir
./baymax mount_dir
```
4. Akses file virtual:
```
cd mountdir
ls          # Akan muncul Baymax.jpeg
cat Baymax.jpeg > output.jpeg   # Membaca hasil gabungan fragmen
```
5. Unmount jika sudah selesai:
```
fusermount3 -u mountdir
```

## 🧬 Alur Program
Mounting filesystem:
- Program FUSE dimulai dan menyediakan satu file virtual: Baymax.jpeg.
Saat file dibaca (read):
- Program membuka fragmen satu per satu dari /relics/Baymax.jpeg.000 sampai .013.
- Semua isi fragmen digabung dan dikembalikan sebagai isi file.
- Aktivitas dicatat di activity.log.
Saat file ditulis (write):
- Isi file yang ditulis dibagi per 1024 byte, dan disimpan sebagai fragmen di folder /relics.
- Misalnya: 15 KB akan disimpan sebagai Baymax.jpeg.000 sampai Baymax.jpeg.013.
- Semua nama fragmen dicatat ke log.
Saat file dihapus (unlink):
- Semua fragmen dengan nama Baymax.jpeg.### dihapus dari folder ./relics.
- Fragmen yang dihapus dicatat ke log.

## 📄 Contoh Log (activity.log)
```
[2025-05-17 12:35:10] READ: Baymax.jpeg
[2025-05-17 12:36:22] WRITE: Baymax.jpeg -> Baymax.jpeg.000, Baymax.jpeg.001, ...
[2025-05-17 12:37:01] DELETE: Baymax.jpeg.000 Baymax.jpeg.001 ...
```

# SOAL 3
## ✏️ Pembuat: Tiara Putri Prasetya (5027241013)

## 📂 Ringkasan
Program ini adalah filesystem virtual menggunakan FUSE, dijalankan di dalam container Docker. Tujuannya untuk memanipulasi file dan direktori dengan fitur keamanan tambahan:
- Mendeteksi file mencurigakan (berisi nafis atau kimcun)
- Enkripsi/Deskripsi file menggunakan ROT13
- Logging aktivitas filesystem
- Reverse nama file mencurigakan di readdir

## Fungsi Utama pada `antink.c`
1. `void rot13(char *s)`
- Fungsi ini mengenkripsi atau mendekripsi string menggunakan ROT13:
- ROT13 adalah teknik substitusi huruf: A <> N, B <> O, dst.
2. `int is_malicious(const char *name)`
- Fungsi ini memeriksa apakah nama file mengandung kata nafis atau kimcun.
- `readdir()` = digunakan untuk membalik nama file mencurigakan.
- `read()` = digunakan untuk mencatat log jika file mencurigakan dibaca.
3. `void reverse_str(char *str)`
- Fungsi ini untuk membalikan string secara manual.
- Contoh = nafis.csv menjadi vsc.sifan
4. `void log_event(const char *msg)`
- Fungsi ini dipakai di semua operasi (getattr, readdir, open, read)
- Fungsi ini untuk mencatat aktivitas ke file log `/var/log/it24.log` dengan format:
```
[dd-mm-YYYY][HH:MM:SS] - <pesan>
```

## 🧠 Fungsi Utama pada operasi FUSE
1. `do_getattr`
Dipanggil saat sistem ingin melihat atribut (ukuran, waktu akses) file. Yaitu dengan mengambil atribut file asli di /it24_host dan mencatat log [GETATTR] Path: ...
2. `do_readdir`
Dipanggil saat direktori dibuka (misalnya saat ls).Yaitu dengan membaca semua file dari /it24_host dan jika ada file mencurigakan (mengandung nafis atau kimcun), nama file dibalik dan dicatat di log [READDIR] dan [REVERSED]
3. `do_open`
Dipanggil saat file dibuka. Yaitu dengan membuka atribut file asli di /it24_host dan mencatat log [OPEN] File: ...
4. `do_read`
Dipanggil saat isi file dibaca.
Jika isi file mencurigakan:
- Baca isi file secara manual (`fopen`)
- Tidak mengubah isinya, hanya log [READ MALICIOUS]
Jika bukan mencurigakan:
- Baca isi file
- Enkripsi hasil baca menggunakan ROT13
- Log [READ]

## 🧠 Fungsi Utama pada Dockerfile & Docker-compose.yml
### - Dockerfile
1. Build image berisi FUSE + kode antink:
2. Gunakan ubuntu:latest
3. Install dependensi FUSE dan gcc
4. Copy `antink.c` dan compile
5. Jalankan dengan FUSE mount ke /antink_mount
### - `docker-compose.yml`
Terdiri dari dua container:
1. antink-server
Container utama yang menjalankan FUSE filesystem
Mount direktori:
- ./host → /it24_host (isi asli file)
- Volume untuk mount (/antink_mount)
- Volume untuk log
Lalu, Jalankan FUSE filesystem di /antink_mount
2. antink-logger
-Container ringan (alpine)
-Memantau log secara real time

## ⚙️ CARA RUN
-Untuk menghentikan dan menghapus container lama
`docker compose down`
-Untuk Build ulang image
`docker compose build`
- Untuk menjalankan container di background
`docker compose up -d`
- Untuk Melihat isi direktori mount
`docker exec antink-server ls /antink_mount`
- Untuk Membaca file biasa
`docker exec antink-server cat /antink_mount/test.txt`
- untuk Membaca file 'berbahaya' (nama mengandung 'kimcun') — akan didekripsi ROT13
`docker exec antink-server cat /antink_mount/kimcun.txt`
- Untuk membaca file 'berbahaya' (mengandung 'nafis') — akan didekripsi ROT13
`docker exec antink-server cat /antink_mount/nafis.csv`
- Untuk Melihat log aktivitas
`docker exec antink-server cat /var/log/it24.log`

## 📂 TESTING FILE
```
echo "Isi file biasa" //text.txt
echo "Nafis sedang mencuri data!"  //nafis.csv
echo "Kimcun sedang menghapus file!" //kimcun.txt
```
