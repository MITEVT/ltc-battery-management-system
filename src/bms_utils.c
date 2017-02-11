#include "state_types.h"
#include "bms_utils.h"

uint16_t Get_Total_Cell_Count(PACK_CONFIG_T *pack_config) {
    uint16_t total_num_cells = 0;
    uint8_t i;
    for (i = 0; i < pack_config->num_modules; i++) {
        total_num_cells += pack_config->module_cell_count[i];
    }
    return total_num_cells;
}

