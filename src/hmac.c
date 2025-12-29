#include "hmac.h"

#include <string.h>
#include <mbedtls/md.h>

/* =========================================================
 * Dok?mana g?re TEST anahtar? (4.1)
 * ?retimde plaintext saklanmamal? (dok?man uyar?s?).
 * ========================================================= */
static const char HMAC_SECRET_KEY_STR[] = "SensecapStm32WL55SecretKey2024";
#define HMAC_KEY_SIZE 32

bool hmac_sha256_compute(const uint8_t *key,
                         size_t key_len,
                         const uint8_t *data,
                         size_t data_len,
                         uint8_t out_hash[HMAC_SHA256_SIZE_BYTES])
{
    if (!key || !data || !out_hash) {
        return false;
    }

    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        return false;
    }

    int ret = mbedtls_md_hmac(md_info,
                             key, key_len,
                             data, data_len,
                             out_hash);

    return (ret == 0);
}

bool bytes_to_hex_string(const uint8_t *bytes,
                         size_t len,
                         char *out_hex,
                         size_t out_size)
{
    static const char hex_chars[] = "0123456789abcdef";

    if (!bytes || !out_hex) {
        return false;
    }

    if (out_size < (2 * len + 1)) {
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        out_hex[2 * i]     = hex_chars[(bytes[i] >> 4) & 0x0F];
        out_hex[2 * i + 1] = hex_chars[bytes[i] & 0x0F];
    }

    out_hex[2 * len] = '\0';
    return true;
}

/* =========================================================
 * (Opsiyonel) Dok?mandaki kullan?m desenini kolayla?t?ran helper
 * - ?imdilik main.c kullanm?yor; ileride payload ile birle?tirece?iz.
 * ========================================================= */
bool hmac_sha256_compute_doc_key(const uint8_t *data,
                                 size_t data_len,
                                 uint8_t out_hash[HMAC_SHA256_SIZE_BYTES])
{
    return hmac_sha256_compute((const uint8_t *)HMAC_SECRET_KEY_STR,
                               HMAC_KEY_SIZE,
                               data,
                               data_len,
                               out_hash);
}
