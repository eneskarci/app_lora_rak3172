#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* =========================================================
 * 4. GÜVENLİK (HMAC-SHA256)
 *
 * - HMAC çıktısı : 32 byte
 * - Hex string   : 64 karakter (null terminator hariç)
 * ========================================================= */

#define HMAC_SHA256_SIZE_BYTES   32
#define HMAC_SHA256_HEX_SIZE     64   /* 32 byte -> 64 hex karakter */

/**
 * @brief HMAC-SHA256 hesaplar
 *
 * @param key       Secret key bytes
 * @param key_len   Key length
 * @param data      Input data bytes
 * @param data_len  Input length
 * @param out_hash  Output buffer (32 bytes)
 * @return true     Başarılı
 * @return false    Hata
 */
bool hmac_sha256_compute(const uint8_t *key,
                         size_t key_len,
                         const uint8_t *data,
                         size_t data_len,
                         uint8_t out_hash[HMAC_SHA256_SIZE_BYTES]);

/**
 * @brief Binary -> hex string (küçük harf)
 *
 * @param bytes     Input binary
 * @param len       Input length
 * @param out_hex   Output buffer (en az 2*len + 1)
 * @param out_size  Output buffer size
 * @return true     Başarılı
 * @return false    Hata
 */
bool bytes_to_hex_string(const uint8_t *bytes,
                         size_t len,
                         char *out_hex,
                         size_t out_size);
