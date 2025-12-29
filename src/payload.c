#include "payload.h"
#include <stdio.h>
#include <string.h>

/* =========================================================
 * Payload data part: "T:%.1f,H:%.1f"
 * ========================================================= */
bool payload_build_data_part(float temperature,
                             float humidity,
                             char *out_buf,
                             size_t *out_len)
{
    if (!out_buf || !out_len) {
        return false;
    }

    int len = snprintf(out_buf,
                       DATA_PART_MAX_SIZE,
                       "T:%.1f,H:%.1f",
                       (double)temperature,
                       (double)humidity);

    if (len <= 0 || len >= DATA_PART_MAX_SIZE) {
        return false;
    }

    *out_len = (size_t)len;
    return true;
}

/* =========================================================
 * Final payload: "T:%.1f,H:%.1f#<64hex>"
 * ========================================================= */
bool payload_build_final(const char *data_part,
                          const char *hmac_hex,
                          char *out_buf,
                          size_t *out_len)
{
    if (!data_part || !hmac_hex || !out_buf || !out_len) {
        return false;
    }

    if (strlen(hmac_hex) != HMAC_HEX_SIZE) {
        return false;
    }

    int len = snprintf(out_buf,
                       PAYLOAD_MAX_SIZE,
                       "%s#%s",
                       data_part,
                       hmac_hex);

    if (len <= 0 || len >= PAYLOAD_MAX_SIZE) {
        return false;
    }

    *out_len = (size_t)len;
    return true;
}
