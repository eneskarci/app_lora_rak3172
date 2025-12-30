#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/random/random.h>
#include <zephyr/device.h>
#include <zephyr/drivers/watchdog.h>

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

/* Watchdog timeout (ms) */
#define WDT_TIMEOUT_MS          15000

/* =========================================================
 * Watchdog init
 * ========================================================= */
static const struct device *wdt_dev;
static int wdt_channel_id = -1;

static void watchdog_init(void)
{
#if DT_HAS_ALIAS(wdt0)
    wdt_dev = DEVICE_DT_GET(DT_ALIAS(wdt0));
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(iwdg), okay)
    wdt_dev = DEVICE_DT_GET(DT_NODELABEL(iwdg));
#else
    wdt_dev = NULL;
#endif

    if (!wdt_dev || !device_is_ready(wdt_dev)) {
        LOG_WRN("Watchdog device not ready / not found (continuing without WDT)");
        return;
    }

    struct wdt_timeout_cfg cfg = {
        .window = {
            .min = 0,
            .max = WDT_TIMEOUT_MS,
        },
        .flags = WDT_FLAG_RESET_SOC,
    };

    wdt_channel_id = wdt_install_timeout(wdt_dev, &cfg);
    if (wdt_channel_id < 0) {
        LOG_WRN("wdt_install_timeout failed (%d), continuing without WDT", wdt_channel_id);
        wdt_channel_id = -1;
        return;
    }

    int ret = wdt_setup(wdt_dev, 0);
    if (ret < 0) {
        LOG_WRN("wdt_setup failed (%d), continuing without WDT", ret);
        wdt_channel_id = -1;
        return;
    }

    LOG_INF("Watchdog enabled (timeout=%d ms)", WDT_TIMEOUT_MS);
}

static inline void watchdog_feed(void)
{
    if (wdt_dev && (wdt_channel_id >= 0)) {
        (void)wdt_feed(wdt_dev, wdt_channel_id);
    }
}

/* =========================================================
 * Random float (1 ondalık)
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
    LOG_INF("======================================");

    watchdog_init();

    /* LoRaWAN start:
       - CONFIG_LORAWAN_NVM_SETTINGS=y ise daha önceki context/settings varsa restore eder. */
    ret = lorawan_start();
    if (ret < 0) {
        LOG_ERR("lorawan_start failed: %d", ret);
        return 0;
    }

    /* Confirmed mesaj retry sayısını stack’e bildir */
    (void)lorawan_set_conf_msg_tries(UPLINK_MAX_RETRIES);

    struct lorawan_join_config join_cfg = { 0 };
    join_cfg.mode = LORAWAN_ACT_OTAA;
    join_cfg.dev_eui        = (uint8_t *)LORAWAN_DEVICE_EUI;
    join_cfg.otaa.join_eui  = (uint8_t *)LORAWAN_JOIN_EUI;
    join_cfg.otaa.app_key   = (uint8_t *)LORAWAN_APP_KEY;

    bool joined = false;

    for (int i = 1; i <= JOIN_MAX_RETRIES; i++) {
        watchdog_feed();
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
        watchdog_feed();

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
            /* Confirmed uplink */
            ret = lorawan_send(LORAWAN_APP_PORT,
                               (uint8_t *)final_payload,
                               (uint8_t)final_len,
                               LORAWAN_MSG_CONFIRMED);

            if (ret == 0) {
                LOG_INF("Uplink sent");
            } else {
                LOG_WRN("Uplink failed (%d)", ret);
            }
        }

sleep:
        k_sleep(K_SECONDS(TX_INTERVAL_SECONDS));
    }

    return 0;
}
