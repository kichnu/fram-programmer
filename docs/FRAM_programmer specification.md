# FRAM ESP32 Credentials - Specyfikacja Implementacji

## Przegląd Systemu

System przechowuje zaszyfrowane credentials ESP32 w pamięci FRAM z użyciem AES-256-CBC + SHA-256. Dane są programowane offline przez **Beetle RP2350** i odczytywane przez **ESP32** w czasie runtime.

## Hardware

- **FRAM:** 32KB (FM25CL64, MB85RC256V)
- **I2C Address:** 0x50
- **Częstotliwość:** 100kHz
- **Zasilanie:** 3.3V
- **Połączenia:** SDA, SCL, VCC, GND (pull-up 4.7kΩ)

## Struktura Danych FRAM

### Lokalizacja
- **Adres bazowy:** 0x0018 (24 bajty offset)
- **Rozmiar:** 1024 bajty (0x0018 - 0x0417)
- **Zachowanie:** Dane spoza tego zakresu nie są modyfikowane

### FRAMCredentials Structure (1024 bytes)

```cpp
struct __attribute__((packed)) FRAMCredentials {
    uint32_t magic;                    // 0x43524544 ("CRED")
    uint16_t version;                  // 0x0001
    uint8_t  reserved_header[2];       // 0x00, 0x00
    char     device_name[32];          // Plain text, null-terminated
    uint8_t  iv[8];                   // Random IV per session
    uint8_t  encrypted_wifi_ssid[64];      // AES-256-CBC
    uint8_t  encrypted_wifi_password[128]; // AES-256-CBC  
    uint8_t  encrypted_admin_hash[96];     // AES-256-CBC SHA-256
    uint8_t  encrypted_vps_token[160];     // AES-256-CBC
    uint16_t checksum;                     // Sum(bytes 0-495)
    uint8_t  reserved_footer[14];          // 0x00 padding
    uint8_t  expansion[512];               // Future use
};
```

### Field Layout
```
Offset | Size | Field                    | Encryption
-------|------|--------------------------|------------
0      | 4    | magic                    | Plain
4      | 2    | version                  | Plain
6      | 2    | reserved_header          | Plain
8      | 32   | device_name              | Plain
40     | 8    | iv                       | Plain
48     | 64   | encrypted_wifi_ssid      | AES-256-CBC
112    | 128  | encrypted_wifi_password  | AES-256-CBC
240    | 96   | encrypted_admin_hash     | AES-256-CBC
336    | 160  | encrypted_vps_token      | AES-256-CBC
496    | 2    | checksum                 | Plain
498    | 14   | reserved_footer          | Plain
512    | 512  | expansion                | Plain
```

## Algorytm Szyfrowania

### Key Generation
```cpp
String key_material = device_name + 
    "ESP32_WATER_SYSTEM_2024_SECURE_SALT_V1" + 
    "WATER_DOLEWKA_FIXED_SEED_12345";
uint8_t encryption_key[32] = SHA256(key_material);
```

### Encryption Process
1. **Admin Password Hashing:** `admin_hash = SHA256(admin_password)` → hex string
2. **Random IV Generation:** 8 bajtów per session
3. **Data Encryption:** AES-256-CBC z PKCS#7 padding
4. **Field Padding:** Wypełnienie zerami do rozmiaru pola

### AES-256-CBC Parameters
- **Klucz:** 256-bit (32 bajty) z SHA-256
- **IV:** 8 bajtów → rozszerzony do 16 (powtórzenie)
- **Padding:** PKCS#7 do wielokrotności 16 bajtów
- **Mode:** CBC (Cipher Block Chaining)

### PKCS#7 Padding Rules
```cpp
// Jeśli data_len % 16 == 0, dodaj pełny blok padding (16 bajtów)
padding_bytes = (data_len % 16 == 0) ? 16 : (16 - data_len % 16);
for (int i = 0; i < padding_bytes; i++) {
    data[data_len + i] = padding_bytes;
}
```

### Checksum Calculation
```cpp
uint16_t checksum = 0;
for (int i = 0; i < 496; i++) {  // Exclude checksum field
    checksum += fram_data[i];
}
```

## Programowanie FRAM

### Sprzęt
- **Programmer:** Beetle RP2350 (SDA=GPIO4, SCL=GPIO5)
- **Biblioteki:** Adafruit_FRAM_I2C, własne AES+SHA256

### Procedura
1. **Backup:** Zapisz istniejące dane FRAM
2. **Input Validation:** Sprawdź długości i format danych
3. **Encryption:** Szyfruj każde pole osobno
4. **Write:** Zapisz strukturę na 0x0018
5. **Verify:** Odczytaj i zweryfikuj checksum
6. **Test:** Spróbuj dekrypcji wszystkich pól

### CLI Commands
```bash
program      # Interaktywne programowanie
config       # JSON input
verify       # Weryfikacja z dekrypcją
backup       # Hex dump całego FRAM
info         # Status i informacje
test         # Diagnostyka sprzętu
```

### JSON Format
```json
{
  "device_name": "DOLEWKA_001",
  "wifi_ssid": "MyNetwork",
  "wifi_password": "MyPassword",
  "admin_password": "admin123",
  "vps_token": "sha256:abc123..."
}
```

## ESP32 Implementation

### Inicjalizacja
```cpp
bool verifyFRAM() {
    FRAMCredentials creds;
    fram.read(0x0018, (uint8_t*)&creds, sizeof(creds));
    
    if (creds.magic != 0x43524544) return false;
    if (creds.version != 0x0001) return false;
    
    uint16_t calc_checksum = calculateChecksum((uint8_t*)&creds, 496);
    return (creds.checksum == calc_checksum);
}
```

### Dekrypcja
```cpp
bool loadCredentials() {
    FRAMCredentials fram_creds;
    fram.read(0x0018, (uint8_t*)&fram_creds, sizeof(fram_creds));
    
    // Generate key from device_name
    uint8_t key[32];
    generateEncryptionKey(String(fram_creds.device_name), key);
    
    // Decrypt each field with AES-256-CBC
    decryptField(fram_creds.encrypted_wifi_ssid, 64, key, fram_creds.iv, wifi_ssid);
    decryptField(fram_creds.encrypted_wifi_password, 128, key, fram_creds.iv, wifi_password);
    // etc...
}
```

### Obsługa Błędów
- **Magic mismatch:** FRAM niezainicjalizowany
- **Version mismatch:** Niekompatybilna wersja
- **Checksum error:** Korupcja danych
- **Decrypt failure:** Błędny klucz lub uszkodzone dane

## Bezpieczeństwo

### Kluczowe Aspekty
- **Device-specific keys:** Każde urządzenie ma unikalny klucz
- **Random IV:** Nowy IV dla każdej sesji programowania
- **Offline programming:** Credentials nigdy nie są w plain text na ESP32
- **Checksum integrity:** Detekcja korupcji danych

### Zagrożenia
- **Physical access:** Dostęp do FRAM = dostęp do zaszyfrowanych danych
- **Key extraction:** Analiza device_name może ujawnić klucz
- **Side channels:** Możliwe ataki na AES w ESP32

## Troubleshooting

### Typowe Problemy
```
Magic mismatch     → FRAM nie zaprogramowany
Checksum error     → Korupcja danych lub błąd I2C  
Decrypt failure    → Nieprawidłowy device_name
I2C timeout        → Problem z połączeniem FRAM
```

### Diagnostyka
1. **I2C scan:** Sprawdź czy FRAM odpowiada na 0x50
2. **Magic check:** Odczytaj pierwsze 4 bajty z 0x0018
3. **Checksum test:** Przelicz i porównaj checksum
4. **Decrypt test:** Spróbuj zdekryptować krótkie pole

### Recovery
- **Backup restoration:** Przywróć z hex dump
- **Re-programming:** Użyj Beetle RP2350 programmer
- **Factory reset:** Wyczyść sekcję credentials (fill 0x00)

## Kompatybilność

### Tested Hardware
- **ESP32:** DevKit, WROOM, WROVER
- **FRAM:** FM25CL64, MB85RC256V, MB85RC512T
- **Programmer:** Beetle RP2350

### Wersjonowanie
- **v1.0:** Aktualna implementacja
- **Backward compatibility:** Upgrade path przez version field
- **Future fields:** Expansion area (512 bajtów) dla nowych funkcji

---

**Status:** Production Ready  
**Ostatnia aktualizacja:** 2024-12  
**Kompatybilność:** ESP32 + FRAM 32KB+