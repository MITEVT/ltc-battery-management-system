#include "state_types.h"
#include "bms_utils.h"

uint16_t Get_Total_Cell_Count(PACK_CONFIG_T *pack_config) {
	uint16_t total_num_cells = 0;
	int i;
	for (i = 0; i < pack_config->num_modules; i++) {
		total_num_cells += pack_config->num_cells_in_modules[i];
	}
    return total_num_cells;
}

