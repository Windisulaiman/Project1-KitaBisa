# Project1-KitaBisa
Project1 Sistem Mikrokontroler

## Komponen yang Digunakan

| No | Komponen | Pin Arduino | Fungsi |
|---|---|---|---|
| 1 | LDR | A0 | Membaca intensitas cahaya/api |
| 2 | Relay | 10 | Mengaktifkan pompa air |
| 3 | Servo | 9 | Menggerakkan arah semprotan |
| 4 | Buzzer | 3 | Alarm suara |
| 5 | LED Merah | 4 | Indikator bahaya/waspada |
| 6 | LED Hijau | 5 | Indikator aman |
| 7 | LCD I2C 16x2 | SDA = A4, SCL = A5 | Menampilkan status sistem |
| 8 | Tombol Reset | 2 | Reset manual menggunakan interrupt |

## Cara Kerja Sistem

Sistem bekerja dengan membaca nilai sensor LDR secara terus-menerus. Nilai dari sensor LDR kemudian dibandingkan dengan batas atau threshold yang sudah ditentukan di dalam program.

### 1. Kondisi Aman

Jika nilai LDR berada pada kondisi normal, maka sistem menampilkan status **AMAN** pada LCD. LED hijau akan menyala, sedangkan LED merah, buzzer, relay, dan servo dalam keadaan mati atau normal.

### 2. Kondisi Waspada

Jika nilai LDR mulai mendeteksi adanya cahaya/api dengan intensitas tertentu, maka sistem masuk ke kondisi **WASPADA**. Pada kondisi ini, LED merah menyala, buzzer berbunyi pelan, servo berada pada posisi siaga, dan LCD menampilkan status waspada.

### 3. Kondisi Bahaya

Jika nilai LDR melewati batas bahaya, maka sistem masuk ke kondisi **BAHAYA KRITIS**. Relay akan aktif sebagai simulasi pompa air, buzzer berbunyi dengan frekuensi tinggi, LED merah berkedip, dan servo bergerak bolak-balik untuk mensimulasikan gerakan semprotan air.

### 4. Reset Manual

Tombol reset digunakan untuk menghentikan proses pemadaman secara manual. Tombol ini menggunakan interrupt eksternal pada pin 2, sehingga sistem dapat langsung merespons ketika tombol ditekan.
