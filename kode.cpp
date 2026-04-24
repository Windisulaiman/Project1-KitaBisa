
```cpp

#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// --- Pin Assignment ---
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
  digitalWrite(RELAY,    LOW);
  digitalWrite(LEDMERAH, LOW);
  digitalWrite(LEDHIJAU, HIGH);
  noTone(BUZZER);
  servo.write(0);
}

// Fungsi bantu: kedipkan LED merah (untuk level kritis)
void kedipLedMerah(int kali) {
  for (int i = 0; i < kali; i++) {
    digitalWrite(LEDMERAH, HIGH);
    delay(150);
    digitalWrite(LEDMERAH, LOW);
    delay(150);
  }
}

// Fungsi bantu: servo sweep bolak-balik (simulasi semprot air)
void servoSweep(int maxAngle) {
  // Sweep dari 0 ke maxAngle
  for (int pos = 0; pos <= maxAngle; pos += 5) {
    servo.write(pos);   // PWM: kontrol sudut servo
    delay(40);
  }
  // Sweep balik ke 0
  for (int pos = maxAngle; pos >= 0; pos -= 5) {
    if (resetFlag) return;
    servo.write(pos);
    delay(40);
  }
}

// SETUP
void setup() {
  Serial.begin(9600);              // UART: buka komunikasi serial
  Serial.println("=== Sistem Pemadam Api v2.0 ===");

  pinMode(LEDMERAH,  OUTPUT);
  pinMode(LEDHIJAU,  OUTPUT);
  pinMode(RELAY,     OUTPUT);
  pinMode(BUZZER,    OUTPUT);
  pinMode(BTN_RESET, INPUT_PULLUP); // pullup internal, aktif LOW

  servo.attach(SERVOPIN);  // inisialisasi PWM servo
  servo.write(0);

  // Daftarkan interrupt eksternal pin 2
  attachInterrupt(digitalPinToInterrupt(BTN_RESET), ISR_Reset, FALLING);

  // Inisialisasi LCD I2C
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("PEMADAM API");
  lcd.setCursor(0, 1); lcd.print("OTOMATIS v2.0");

  matikanSistem();
  delay(2000);
}

// LOOP — Perulangan utama sistem
void loop() {

  // Cek flag interrupt reset
  if (resetFlag) {
    resetFlag = false;
    matikanSistem();
    Serial.print("["); Serial.print(millis()/1000);
    Serial.println("s] INTERRUPT: Reset manual");
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("RESET MANUAL");
    lcd.setCursor(0,1); lcd.print("Sistem normal");
    delay(2000);
    return;
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
    lcd.clear();    // Bersihkan layar LCD agar tidak ada tampilan sebelumnya
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
    lcd.setCursor(0,0); lcd.print("STATUS: AMAN");
    lcd.setCursor(0,1); lcd.print("LDR:"); lcd.print(ldr);

    matikanSistem();
    delay(2000);
  }

  delay(500);
}
```
