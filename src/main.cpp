// =========================
// 1. DEFINISI BLYNK (PALING ATAS)
// =========================
#define BLYNK_TEMPLATE_ID "TMPL6UZuKoQ2a"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "5JreyI-JXx5dlw3ihx2-I1GUy9gGcKHf"

#define BLYNK_PRINT Serial

// =========================
// 2. INCLUDE LIBRARY
// =========================
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// =========================
// KONFIGURASI PIN & VAR
// =========================
#define SENSOR_PIN 25
#define RELAY_PIN 26
#define BUZZER_PIN 17

#define TARGET_COUNT 12
#define DEBOUNCE_DELAY 50

// WiFi Wokwi
char ssid[] = "iPhone";
char pass[] = "12341234";

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

int counter = 0;
bool lastState = HIGH;
unsigned long lastDebounceTime = 0;

// ===============================================
// FUNGSI KIRIM DATA KE BLYNK

// ===============================================
void sendToBlynk()
{
    if (Blynk.connected())
    {
        Blynk.virtualWrite(V0, counter);                 // Jumlah barang
        Blynk.virtualWrite(V1, digitalRead(RELAY_PIN));  // Status Motor
        Blynk.virtualWrite(V2, digitalRead(BUZZER_PIN)); // Status Buzzer
        Serial.println("Data sent to Blynk");
    }
}

void setup()
{
    Serial.begin(115200);

    // -----------------------------------------------------------
    // 1. INISIALISASI LCD LEBIH AWAL (Agar bisa tampil status)
    // -----------------------------------------------------------
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();

    // Tampilan: SYSTEM START
    lcd.setCursor(0, 0);
    lcd.print("   SYSTEM START   ");
    lcd.setCursor(0, 1);
    lcd.print("   Please Wait... ");
    delay(1500); // Tahan 1.5 detik biar terbaca

    // Setup Pin
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Kondisi Awal Hardware
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer OFF
    digitalWrite(RELAY_PIN, HIGH);  // Motor ON

    Serial.println("System Starting...");

    // -----------------------------------------------------------
    // 2. PROSES KONEKSI WIFI
    // -----------------------------------------------------------
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connect WiFi...");

    Serial.println("Menghubungkan ke WiFi...");
    WiFi.begin(ssid, pass);

    int retry = 0;
    // Loop connect (max 10 detik)
    while (WiFi.status() != WL_CONNECTED && retry < 20)
    {
        delay(500);
        Serial.print(".");
        lcd.setCursor(retry % 16, 1); // Animasi titik di baris 2
        lcd.print(".");
        retry++;
    }

    // -----------------------------------------------------------
    // 3. LOGIKA SETELAH CEK WIFI
    // -----------------------------------------------------------
    if (WiFi.status() == WL_CONNECTED)
    {
        // === JIKA ONLINE ===
        Serial.println("\nWiFi Connected!");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected!");
        delay(1000);

        // Tampilan: Connect Blynk
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Connect Blynk...");

        // Config Blynk
        Blynk.config(BLYNK_AUTH_TOKEN);
        bool result = Blynk.connect();

        if (result)
        {
            lcd.setCursor(0, 1);
            lcd.print("Blynk OK!");
        }
        else
        {
            lcd.setCursor(0, 1);
            lcd.print("Blynk Fail!");
        }
        delay(1500);
    }
    else
    {
        // === JIKA OFFLINE ===
        Serial.println("\nWiFi Gagal! Masuk Mode OFFLINE.");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Failed!");
        lcd.setCursor(0, 1);
        lcd.print("Mode: OFFLINE");

        // Bunyi Beep pendek tanda offline
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        digitalWrite(BUZZER_PIN, HIGH);

        delay(2000); // Tahan tulisan offline 2 detik
    }

    // -----------------------------------------------------------
    // 4. MASUK TAMPILAN UTAMA (PROGRAM DIMULAI)
    // -----------------------------------------------------------
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Barang: 0/");
    lcd.print(TARGET_COUNT);

    // Setup Timer
    timer.setInterval(1000L, sendToBlynk);

    Serial.println("Sistem Ready!");
}

void loop()
{
    // Jalankan fungsi Blynk hanya jika terhubung
    if (Blynk.connected())
    {
        Blynk.run();
    }

    timer.run();

    // Membaca Sensor
    int sensor = digitalRead(SENSOR_PIN);

    if (sensor == LOW && lastState == HIGH && (millis() - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        lastDebounceTime = millis();
        counter++;

        Serial.print("Barang ke-");
        Serial.println(counter);

        lcd.setCursor(8, 0);
        lcd.print(counter);

        // Update ke Blynk Real-time
        if (Blynk.connected())
        {
            Blynk.virtualWrite(V0, counter);
        }

        // LOGIKA TARGET TERCAPAI
        if (counter >= TARGET_COUNT)
        {
            Serial.println("=== TARGET TERCAPAI ===");

            // Matikan Motor
            digitalWrite(RELAY_PIN, LOW);
            if (Blynk.connected())
                Blynk.virtualWrite(V1, 0);

            // Tampilan LCD Target
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TARGET ");
            lcd.print(TARGET_COUNT);
            lcd.print(" OK!");
            lcd.setCursor(0, 1);
            lcd.print("MOTOR BERHENTI");

            // Bunyikan Buzzer
            digitalWrite(BUZZER_PIN, LOW); // ON
            if (Blynk.connected())
                Blynk.virtualWrite(V2, 1);

            // Smart Delay
            unsigned long startDelay = millis();

            // Delay 3 detik buzzer
            while (millis() - startDelay < 3000)
            {
                if (Blynk.connected())
                {
                    Blynk.run();
                }
            }

            digitalWrite(BUZZER_PIN, HIGH); // OFF
            if (Blynk.connected())
                Blynk.virtualWrite(V2, 0);

            // Delay 2 detik sebelum reset
            startDelay = millis();
            while (millis() - startDelay < 2000)
            {
                if (Blynk.connected())
                {
                    Blynk.run();
                }
            }

            // Reset Sistem
            counter = 0;
            digitalWrite(RELAY_PIN, HIGH); // Motor ON lagi
            if (Blynk.connected())
                Blynk.virtualWrite(V1, 1);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Barang: 0/");
            lcd.print(TARGET_COUNT);

            if (Blynk.connected())
                Blynk.virtualWrite(V0, 0);

            Serial.println("Sistem direset!");
        }
    }

    lastState = sensor;
}