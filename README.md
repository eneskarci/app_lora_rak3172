# app_lora_rak3172

RAK3172 (STM32WL) tabanlı LoRaWAN Class A sensör düğümü.
Proje Zephyr RTOS 4.3.0 kullanılarak geliştirilmiştir.

## Özellikler
- EU868 LoRaWAN bölgesi
- OTAA (DevEUI, JoinEUI, AppKey)
- Confirmed uplink + retry mekanizması
- Payload bütünlüğü için HMAC-SHA256
- Power management (uplink sonrası sleep)
- ChirpStack uyumlu

## Payload Formatı
T:%.1f,H:%.1f#<64_hex_hmac>

## Build
```bash
west build -b rak3172 -p always
