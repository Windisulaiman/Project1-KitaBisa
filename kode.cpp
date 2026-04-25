
```cpp

#include <LiquidCrystal_I2C.h> //Import library LCD I2C
#include <Servo.h> //Import library untuk kontrol motor servo

// --- Deklarassi pin ---
#define BUZZER    3
#define LEDMERAH  4
#define LEDHIJAU  5
#define SERVOPIN  9
#define RELAY     10
#define LDRPIN    A0
#define BTN_RESET 2   // pin interrupt eksternal

// --- Threshold 3 Level ---
#define BAHAYA   100   // LDR < 100  → bahaya kritis
#define WASPADA  300   // LDR < 300  → waspada
                        // LDR >= 300 → aman

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C: SDA=A4, SCL=A5
Servo servo;

volatile bool resetFlag = false;

// ISR — Interrupt Eksternal (pin 2, FALLING)
// Dipanggil otomatis saat tombol reset ditekan
void ISR_Reset() {
  resetFlag = true;
}

// Fungsi bantu: matikan semua sistem pemadaman
void matikanSistem() {
  digitalWrite(RELAY,    LOW); //Mematikan relay 
  digitalWrite(LEDMERAH, LOW); 
  digitalWrite(LEDHIJAU, HIGH);
  noTone(BUZZER); // menghentikan bunyi buzzer
  servo.write(0);
}
//kondisi lampu ketika dilakukan reset
void resetSistem() {
  digitalWrite(RELAY,    LOW);
  digitalWrite(LEDMERAH, LOW);
  digitalWrite(LEDHIJAU, LOW);   // Semua LED OFF dulu
  noTone(BUZZER);
  servo.write(0);
}
// Fungsi bantu: kedipkan LED merah (untuk level kritis)
void kedipLedMerah(int kali) {
  for (int i = 0; i < kali; i++) {
    //perulangan kedip lampu
    digitalWrite(LEDMERAH, HIGH);
    delay(150); // durasi nyala
    digitalWrite(LEDMERAH, LOW);
    delay(150); // durasi mati
  }
}

// Fungsi bantu: servo sweep bolak-balik (simulasi semprot air)
void servoSweep(int maxAngle) {
  // Sweep dari 0 ke maxAngle
  for (int pos = 0; pos <= maxAngle; pos += 5) {
    servo.write(pos);   // PWM: kontrol sudut servo
    delay(40);  //durasi gerak servo
  }
  // Sweep balik ke 0
  for (int pos = maxAngle; pos >= 0; pos -= 5) {
    if (resetFlag) return;
    servo.write(pos); // durasi gerak servo 
    delay(40);
  }
}

// SETUP
void setup() {
  Serial.begin(9600); // Memulai komunikasi serial dengan baud rate 9600
  Serial.println("=== Sistem Pemadam Api v2.0 ==="); // Menampilkan teks awal di Serial Monitor

  pinMode(LEDMERAH,  OUTPUT); // Mengatur pin LED merah sebagai output
  pinMode(LEDHIJAU,  OUTPUT); // Mengatur pin LED hijau sebagai output
  pinMode(RELAY,     OUTPUT); // Mengatur pin relay (pompa) sebagai output
  pinMode(BUZZER,    OUTPUT); // Mengatur pin buzzer sebagai output
  pinMode(BTN_RESET, INPUT_PULLUP); // Mengatur tombol reset sebagai input dengan pull-up internal, Default HIGH, ditekan jadi LOW

  servo.attach(SERVOPIN);  // inisialisasi PWM servo
  servo.write(0); // Mengatur posisi awal servo ke 0 derajat (posisi awal)

  // Daftarkan interrupt eksternal pin 2
  attachInterrupt(digitalPinToInterrupt(BTN_RESET), ISR_Reset, FALLING);

  // Inisialisasi LCD I2C
  lcd.init(); // Menginisialisasi LCD agar siap digunakan (memulai komunikasi dengan modul LCD)
  lcd.backlight(); // Menyalakan lampu backlight pada LCD supaya layar terlihat terang
  lcd.clear(); // Menghapus semua tampilan sebelumnya di LCD (layar jadi kosong)
  lcd.setCursor(0, 0); // Mengatur posisi kursor ke kolom 0, baris 0 (pojok kiri atas)
  lcd.print("PEMADAM API");  // Menampilkan teks "PEMADAM API" di posisi kursor saat ini
  lcd.setCursor(0, 1); // Memindahkan kursor ke kolom 0, baris 1 (baris kedua)
  lcd.print("OTOMATIS v2.0"); // Menampilkan teks "OTOMATIS v2.0" di baris kedua LCD

  matikanSistem(); // Memanggil fungsi untuk mematikan sistem (misalnya mematikan relay, pompa, buzzer, dll)
  delay(2000); // Memberi jeda selama 2000 milidetik (2 detik) sebelum program lanjut
}

// LOOP — Perulangan utama sistem
void loop() {

  // Mengecek apakah tombol reset manual ditekan melalui interrupt
  if (resetFlag) {
    resetFlag = false; // Mengembalikan flag reset agar tidak terus aktif
    resetSistem();   // Mematikan semua sistem (relay OFF, buzzer mati, LED normal, servo ke posisi awal)
    Serial.print("["); Serial.print(millis()/1000);   // Menampilkan waktu (detik sejak Arduino menyala) dan pesan reset di Serial Monitor
    Serial.println("s] INTERRUPT: Reset manual");  // Menampilkan keterangan bahwa reset manual terjadi
    lcd.clear();  // Membersihkan tampilan LCD  // Membersihkan tampilan LCD
    lcd.setCursor(0,0); lcd.print("RESET MANUAL");  // Menampilkan pesan reset di baris pertama LCD
    lcd.setCursor(0,1); lcd.print("Sistem normal");   // Mengatur kursor LCD pada kolom 0 baris 1 dan Menampilkan bahwa sistem kembali normal
    delay(2000);   // Memberi jeda 2 detik agar pesan terbaca
    return;   // Menghentikan loop saat ini dan kembali ke awal loop berikutnya
  }

  // Baca ADC dari sensor LDR
  int ldr = analogRead(LDRPIN);
  unsigned long t = millis() / 1000;

  Serial.print("["); Serial.print(t); Serial.print("s] LDR=");
  Serial.print(ldr); Serial.print(" | ");

  // PERCABANGAN 3 LEVEL
  if (ldr < BAHAYA) {
    // === LEVEL 1: BAHAYA KRITIS ===
    Serial.println("BAHAYA KRITIS!");
    
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("!! BAHAYA !!");
    lcd.setCursor(0,1); lcd.print("LDR:"); lcd.print(ldr);
    
    digitalWrite(LEDHIJAU, LOW);   // ketika cahaya naik maka led hijau akan mati 
    digitalWrite(RELAY, HIGH);     // pompa ON
    tone(BUZZER, 1500);            // PWM audio: frekuensi tinggi
    kedipLedMerah(3);              // LED kedip cepat (perulangan)

    // Servo sweep penuh 90 derajat (simulasi semprot menyapu)
    servoSweep(90);

    // Countdown pemadaman di LCD menggunakan for loop
    for (int i = 5; i > 0; i--) {
      if (resetFlag) break;
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Memadamkan...");
      lcd.setCursor(0,1); lcd.print("Selesai dalam: "); lcd.print(i);
      Serial.print("  Countdown: "); Serial.println(i);
      servoSweep(90);               // sweep tiap detik
      delay(200);
    }

    matikanSistem();  // Matikan semua sistem (relay, buzzer, LED, servo kembali ke posisi awal)
    Serial.println("  Pemadaman selesai.");  // Tampilkan pesan di Serial Monitor bahwa proses pemadaman sudah selesai
    lcd.clear();  // Bersihkan layar LCD agar tidak ada tampilan sebelumnya
    lcd.setCursor(0,0); lcd.print("API PADAM");  // Tampilkan pesan bahwa api sudah berhasil dipadamkan
    lcd.setCursor(0,1); lcd.print("Sistem normal");  // Tampilkan bahwa sistem kembali ke kondisi normal
    delay(3000);  // Beri jeda 3 detik agar pengguna bisa membaca pesan di LCD

  } else if (ldr < WASPADA) {
    // === LEVEL 2: WASPADA ===
    Serial.println("WASPADA");

    lcd.clear();
    lcd.setCursor(0,0); lcd.print("WASPADA!");
    lcd.setCursor(0,1); lcd.print("LDR:"); lcd.print(ldr);

    digitalWrite(LEDMERAH, HIGH);  // LED merah nyala statis
    digitalWrite(LEDHIJAU, LOW);
    digitalWrite(RELAY,    LOW);   // pompa masih OFF
    tone(BUZZER, 500);             // PWM audio: frekuensi rendah
    servo.write(45);               // PWM: posisi siaga setengah
    delay(2000);

  } else {
    // === LEVEL 3: AMAN ===
    Serial.println("AMAN");

    lcd.clear();
    lcd.setCursor(0,0); lcd.print("STATUS: AMAN"); // Pindah ke baris 1 kolom 0, tampilkan "STATUS: AMAN"
    lcd.setCursor(0,1); lcd.print("LDR:"); lcd.print(ldr);  // Pindah ke baris 2 kolom 0, tampilkan nilai LDR saat ini

    matikanSistem(); //memastikan semua aktuator mati (relay, buzzer, servo, LED)
    delay(2000); //waktu tunggu sebelum sensor membaca lagi
  }

  delay(500);
}
```
