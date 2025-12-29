#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/random/random.h>

#include "lorawan_keys.h"
#include "payload.h"
#include "hmac.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* =========================================================
 * Sabitler
 * ========================================================= */
#define LORAWAN_APP_PORT        1
#define TX_INTERVAL_SECONDS     600

#define JOIN_RETRY_INTERVAL_MS  60000
#define JOIN_MAX_RETRIES        10

#define UPLINK_MAX_RETRIES      3
#define UPLINK_RETRY_DELAY_MS   5000

#define TEMP_MIN_C              -40.0f
#define TEMP_MAX_C               85.0f
#define HUM_MIN_PERCENT           0.0f
#define HUM_MAX_PERCENT         100.0f

/* =========================================================
 * Random float (1 ondal?k)
 * ========================================================= */
static float random_float_1dp(float min, float max)
{
    uint32_t r = sys_rand32_get() % 1001;
    float value = min + ((float)r / 1000.0f) * (max - min);
    return ((int)(value * 10.0f + 0.5f)) / 10.0f;
}

int main(void)
{
    int ret;

    LOG_INF("======================================");
    LOG_INF(" app_lora_rak3172 starting");
    LOG_INF(" Warning-free build");
    LOG_INF("======================================");

    ret = lorawan_start();
    if (ret < 0) {
        LOG_ERR("lorawan_start failed: %d", ret);
        return 0;
    }

    struct lorawan_join_config join_cfg = { 0 };
    join_cfg.mode = LORAWAN_ACT_OTAA;
    join_cfg.dev_eui        = (uint8_t *)LORAWAN_DEVICE_EUI;
    join_cfg.otaa.join_eui  = (uint8_t *)LORAWAN_JOIN_EUI;
    join_cfg.otaa.app_key   = (uint8_t *)LORAWAN_APP_KEY;

    bool joined = false;

    for (int i = 1; i <= JOIN_MAX_RETRIES; i++) {
        LOG_INF("Join attempt %d/%d", i, JOIN_MAX_RETRIES);

        ret = lorawan_join(&join_cfg);
        if (ret == 0) {
            LOG_INF("Join successful");
            joined = true;
            break;
        }

        LOG_WRN("Join failed (%d), retry in %d ms",
                ret, JOIN_RETRY_INTERVAL_MS);
        k_sleep(K_MSEC(JOIN_RETRY_INTERVAL_MS));
    }

    while (1) {

        float temperature = random_float_1dp(TEMP_MIN_C, TEMP_MAX_C);
        float humidity    = random_float_1dp(HUM_MIN_PERCENT, HUM_MAX_PERCENT);

        LOG_INF("Sensor data: T=%.1f C, H=%.1f %%",
                (double)temperature,
                (double)humidity);

        char data_part[DATA_PART_MAX_SIZE];
        size_t data_len;

        if (!payload_build_data_part(temperature, humidity,
                                     data_part, &data_len)) {
            goto sleep;
        }

        uint8_t hmac_bin[HMAC_SHA256_SIZE_BYTES];
        char hmac_hex[HMAC_SHA256_HEX_SIZE + 1];

        hmac_sha256_compute(
            (const uint8_t *)"SensecapStm32WL55SecretKey2024",
            32,
            (const uint8_t *)data_part,
            data_len,
            hmac_bin);

        bytes_to_hex_string(hmac_bin,
                            HMAC_SHA256_SIZE_BYTES,
                            hmac_hex,
                            sizeof(hmac_hex));

        char final_payload[PAYLOAD_MAX_SIZE];
        size_t final_len;

        payload_build_final(data_part,
                            hmac_hex,
                            final_payload,
                            &final_len);

        if (joined) {
            for (int a = 1; a <= UPLINK_MAX_RETRIES; a++) {
                ret = lorawan_send(LORAWAN_APP_PORT,
                                   final_payload,
                                   final_len,
                                   true);

                if (ret == 0) {
                    break;
                }

                k_sleep(K_MSEC(UPLINK_RETRY_DELAY_MS));
            }
        }

sleep:
        k_sleep(K_SECONDS(TX_INTERVAL_SECONDS));
    }

    return 0;
}
