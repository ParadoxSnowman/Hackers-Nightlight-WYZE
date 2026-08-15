#include <Arduino.h>
#include <components/system_manager.h>
#include <components/wsl_bypasser/wsl_bypasser.h>
#include <components/frame_analyzer/frame_analyzer_parser.h>
#include <components/hccapx_serializer/hccapx_serializer.h>
#include <components/pcap_serializer/pcap_serializer.h>
#include <components/webserver/webserver.h>
#include <components/wifi_controller/wifi_controller.h>
#include "components/bp5758d.h"
#include <WiFi.h>
#include "main/attack.h"

static const char *const TAG = "tuya-globe";

void SystemManager::SetupSystem() {
    // --- LED driver (BP5758D) ---
    bulb.begin();
    bulb.set(0, 0, 0, 240, 0);   // warm white at boot (matches stock)

    // --- Wi-Fi / capture components (unchanged) ---
    wslBypasserInterface    = new wslbypasser();
    frameAnalyzerInterface  = new frame_analyzer();
    hccapxInterface         = new hccapx_serializer();
    pcapInterface           = new pcap_serializer();
    webinterface            = new WebServer();
    wificontrollerInterface = new wifi_controller();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wificontrollerInterface->wifictl_mgmt_ap_start();
    attack_init();
    webinterface->webserver_run();
}

void SystemManager::loop() {
    // TODO;
}
