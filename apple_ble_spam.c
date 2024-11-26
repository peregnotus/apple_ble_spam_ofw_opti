#include <string.h>
#include <stdint.h>

#define TAG "FuriHalBt"
#define BLE_STATUS_TIMEOUT 0xFFU
#define BLE_CMD_MAX_PARAM_LEN 255
#define START_ADDR 0x8000140
#define END_ADDR 0x80800ec
#define HCI_SEND_REQ_ADDR 0x080161e8
#define TARGET_SEQUENCE 0x33680446
#define SEQUENCE_OFFSET 6

typedef uint8_t tBleStatus;

typedef struct __attribute__((packed, aligned(2))) {
    uint8_t Adv_Data_Length;
    uint8_t Adv_Data[BLE_CMD_MAX_PARAM_LEN - 1];
} aci_gap_additional_beacon_set_data_cp0;

typedef struct __attribute__((packed, aligned(2))) {
    uint16_t Adv_Interval_Min;
    uint16_t Adv_Interval_Max;
    uint8_t Adv_Channel_Map;
    uint8_t Own_Address_Type;
    uint8_t Own_Address[6];
    uint8_t PA_Level;
} aci_gap_additional_beacon_start_cp0;

struct hci_request {
    uint16_t ogf;
    uint16_t ocf;
    int event;
    void* cparam;
    int clen;
    void* rparam;
    int rlen;
};

static int (*napi_hci_send_req)(struct hci_request* p_cmd, uint8_t async) = NULL;

void* Osal_MemCpy(void* dest, const void* src, unsigned int size) {
    return memcpy(dest, src, size);
}

void* Osal_MemSet(void* ptr, int value, unsigned int size) {
    return memset(ptr, value, size);
}

uintptr_t* scan_memory_for_sequence(uint32_t sequence) {
    uint8_t* addr;
    uint8_t* target_bytes = (uint8_t*)&sequence;

    for (addr = (uint8_t*)START_ADDR; addr < (uint8_t*)END_ADDR - 3; addr++) {
        if (memcmp(addr, target_bytes, 4) == 0) {
            return (uintptr_t*)(addr - SEQUENCE_OFFSET);
        }
    }
    return (uintptr_t*)HCI_SEND_REQ_ADDR;
}

tBleStatus aci_gap_additional_beacon_start(
    uint16_t Adv_Interval_Min,
    uint16_t Adv_Interval_Max,
    uint8_t Adv_Channel_Map,
    uint8_t Own_Address_Type,
    const uint8_t* Own_Address,
    uint8_t PA_Level) {
    
    struct hci_request rq;
    uint8_t cmd_buffer[BLE_CMD_MAX_PARAM_LEN] = {0};
    aci_gap_additional_beacon_start_cp0* cp0 = (aci_gap_additional_beacon_start_cp0*)cmd_buffer;
    tBleStatus status = 0;

    cp0->Adv_Interval_Min = Adv_Interval_Min;
    cp0->Adv_Interval_Max = Adv_Interval_Max;
    cp0->Adv_Channel_Map = Adv_Channel_Map;
    cp0->Own_Address_Type = Own_Address_Type;
    memcpy(cp0->Own_Address, Own_Address, 6);
    cp0->PA_Level = PA_Level;

    memset(&rq, 0, sizeof(rq));
    rq.ogf = 0x3f;
    rq.ocf = 0x0b0;
    rq.cparam = cmd_buffer;
    rq.clen = sizeof(aci_gap_additional_beacon_start_cp0);
    rq.rparam = &status;
    rq.rlen = 1;

    if (!napi_hci_send_req || napi_hci_send_req(&rq, 0) < 0) return BLE_STATUS_TIMEOUT;
    return status;
}

// Weitere Funktionen ähnlich anpassen.
