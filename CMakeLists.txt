cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_lora_rak3172)

target_sources(app PRIVATE
  src/main.c
  src/payload.c
  src/hmac.c
)
