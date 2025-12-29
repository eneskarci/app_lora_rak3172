#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* =========================================================
 * 3. APPLICATION PAYLOAD FORMAT (Dok?mana birebir)
 *
 * Data Part : "T:%.1f,H:%.1f"
 * Delimiter : "#"
 * HMAC Part : 64 hex char (32 byte SHA256)
 *
 * Final     : "T:25.3,H:60.5#<64hex>"
 * ========================================================= */

#define TEMP_FORMAT_STRING      "T:%.1f"
#define HUM_FORMAT_STRING       "H:%.1f"
#define PAYLOAD_FORMAT_STRING  "T:%.1f,H:%.1f#%s"

/* Boyut s?n?rlar? (dok?mana g?re) */
#define DATA_PART_MAX_SIZE      32
#define HMAC_HEX_SIZE           64
#define PAYLOAD_MAX_SIZE        128

/* =========================================================
 * API
 * ========================================================= */

/**
 * @brief Payload olu?turur (HMAC hari?)
 *
 * @param temperature   S?cakl?k (?C)
 * @param humidity      Nem (%)
 * @param out_buf       ??k?? buffer
 * @param out_len       ??k?? uzunlu?u
 * @return true         Ba?ar?l?
 * @return false        Hata
 */
bool payload_build_data_part(float temperature,
                             float humidity,
                             char *out_buf,
                             size_t *out_len);

/**
 * @brief Final payload olu?turur
 *
 * @param data_part     "T:xx.x,H:yy.y"
 * @param hmac_hex      64 hex karakter
 * @param out_buf       ??k?? buffer
 * @param out_len       ??k?? uzunlu?u
 * @return true         Ba?ar?l?
 * @return false        Hata
 */
bool payload_build_final(const char *data_part,
                          const char *hmac_hex,
                          char *out_buf,
                          size_t *out_len);
