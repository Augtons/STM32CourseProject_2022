#include "smg.h"

const uint8_t smg_duan[] = {
    0xFC,        // 0
    0x60,        // 1
    0xDA,        // 2
    0xF2,        // 3
    0x66,        // 4
    0xB6,        // 5
    0xBE,        // 6
    0xE0,        // 7
    0xFE,        // 8
    0xF6,        // 9
};

void smg_init(hc138_t *hc138, hc595_t *hc595, smg_t *ret_smg) {
    smg_t smg = {
        .hc138 = *hc138,
        .hc595 = *hc595,
        .buf = {0},
        .flash_pointer = 0
    };
    *ret_smg = smg;
}

void smg_from_string(smg_t *smg, const char *str) {
    char *pointer = (char*)str;

    for (int i = 0; i < 8; pointer++) {
        switch (*pointer) {
            case '_':
                smg->buf[i] = 0x00;
                break;
            case '0' ... '9':
                smg->buf[i] = smg_duan[*pointer - '0'];
                break;
            case '.':
                if (i > 0) {
                    smg->buf[i-1] |= 0x1;
                }
                break;
            default:
                break;
        }
        if (*pointer != '.') {
            i++;
        }
    }
}

void smg_test(smg_t *smg) {
    smg_from_string(smg, "_3.14_15.9");
}

void smg_flush(smg_t *smg) {
		if (smg->flash_pointer == 8) {
        smg->flash_pointer = 0;
    }
		uint8_t i = smg->flash_pointer;
    hc595_show(&smg->hc595, 0x00);
    hc138_turn(&smg->hc138, i);
    hc595_show(&smg->hc595, smg->buf[i]);
		smg->flash_pointer ++;
}
