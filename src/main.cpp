#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SENSOR_PIN 25
#define RELAY_PIN 26
#define BUZZER_PIN 17

// Definisikan target hitungan
#define TARGET_COUNT 12
// Atur waktu debouncing dalam milidetik
#define DEBOUNCE_DELAY 50

LiquidCrystal_I2C lcd(0x27, 16, 2);

int counter = 0;
bool lastState = HIGH;
unsigned long lastDebounceTime = 0;

void setup() {
    Serial.begin(115200);
    
    // Setup pins
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    // ===================================
    // KOREKSI: BUZZER LOW LEVEL TRIGGER (Active-LOW)
    // Atur ke HIGH agar buzzer MATI saat startup
    digitalWrite(BUZZER_PIN, HIGH);    // Buzzer MATI (OFF) saat startup
    // ===================================
    
    // Motor ON (Diasumsikan Active-HIGH: HIGH = ON)
    digitalWrite(RELAY_PIN, HIGH);    
    
    // Setup LCD 
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Barang: 0/");
    lcd.print(TARGET_COUNT);
    
    Serial.println("Sistem Ready");
    Serial.println("Buzzer: Active-LOW (HIGH=OFF, LOW=ON)");
    
    delay(1000);
}

void loop() {
    int sensor = digitalRead(SENSOR_PIN);
    
    // Cek Transisi dari HIGH ke LOW (Barang baru terdeteksi)
    if (sensor == LOW && lastState == HIGH && (millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        
        lastDebounceTime = millis(); 
        
        counter++;
        
        Serial.print("Barang ke-");
        Serial.println(counter);
        
        // Update LCD
        lcd.setCursor(8, 0);
        lcd.print(counter);
        
        // Jika mencapai TARGET_COUNT barang
        if (counter >= TARGET_COUNT) {
            Serial.println("=== TARGET TERCAPAI ===");
            
            // 1. Stop motor (LOW = Motor OFF)
            digitalWrite(RELAY_PIN, LOW);
            Serial.println("Motor STOP");
            
            // 2. Update LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TARGET ");
            lcd.print(TARGET_COUNT);
            lcd.print(" OK!");
            lcd.setCursor(0, 1);
            lcd.print("MOTOR BERHENTI");
            
            // 3. ===== AKTIVASI BUZZER (LOW = Buzzer ON) =====
            Serial.println("Buzzer ON (3 detik)...");
            digitalWrite(BUZZER_PIN, LOW);   // LOW untuk ON (Active-LOW)
            delay(3000);                    // 3 detik
            digitalWrite(BUZZER_PIN, HIGH);  // HIGH untuk OFF
            Serial.println("Buzzer OFF");
            
            // 4. Tunggu 2 detik
            delay(2000);
            
            // 5. Reset sistem
            Serial.println("Reset sistem...");
            counter = 0;
            // Motor ON kembali (HIGH = Motor ON)
            digitalWrite(RELAY_PIN, HIGH);   
            
            // 6. Update LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Barang: 0/");
            lcd.print(TARGET_COUNT);
            
            Serial.println("Sistem direset, siap lagi!");
            Serial.println("=======================");
        }
    }
    
    lastState = sensor;
    delay(10);
}