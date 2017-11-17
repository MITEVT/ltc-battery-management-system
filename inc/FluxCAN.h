#ifndef __FLUXCAN_H__
#define __FLUXCAN_H__

#define BMS_HEARTBEAT_PERIOD 		1000
#define BMS_ERRORS_PERIOD			1000
#define BMS_CELL_TEMPS_PERIOD		1000
#define BMS_PACK_STATUS_PERIOD		1000


//special high priority messages
#define SAMPLE_BITCHES				0b010
//window messages are ID batta, battb, battc, battd
#define WINDOW_MSG_DLC				2

// Driver controls CAN base address and packet offsets
#define DC_CAN_BASE					0x500
#define DC_DRIVE					0x1
#define DC_POWER					0x2
#define DC_RESET					0x3
#define DC_SWITCH					0x5

#define DRIVER_CONTROL_DLC			2


/*
	Tritium Motor Controller 
	Both motor controllers should have the same driver control base address (0x500) programmed into MC
	They should have different base address for telemetry read out Motor 1 is 0x400,  Motor 2 is 0x300 (newer)
*/

//4bytes - bus current
//4bytes - bus voltage
#define MOTORDATAAV_MSG_ID_2		0x302

//4bytes - speed
//4bytes - rpm
#define MOTORDATASP_MSG_ID_2		0x303

//4bytes - IPM Phase B Temp
//4bytes - DSP Board Temp
#define MOTORDATATEMP_MSG_ID_2		0x30c

//31 - 16 bits: Error Flags
//15 - 0 bits: Limit Flags
#define MOTORFLAG_MSG_ID_2			0x301


//MPPT Drivetek
//8 bytes for now, not specified
#define MPPT_MSG_ID 				0b01000000000 
//MPPT1 request frame 0b11100010001
//MPPT1 answer frame  0b11101110001
//MPPT2 request frame 0b11100010011
//MPPT2 answer frame  0b11101110011

#define MPPT_MSG_DLC				8

//2 bytes - array deci-amps
//2 bytes - array deci-amp-hours
#define ARRAY_EMETER_MSG_ID			0x700 
#define ARRAY_EMETER_MSG_DLC		4

//battery messages
#define CELLOFFSETVOLTAGE	20
//the message ID should be set to ((BATTX<<BATTBITS)|VOLTAGES47)
#define DATA_MSG_DLC	8
#define BATTBITS		4
#define BATTBITMASK		0b1111
#define BATTA			0b100
#define BATTB			0b101
#define BATTC			0b110
#define BATTD			0b111

#define NUMBATTMSGS		7
//Bits 1,2,3 indicate index position in the data matrix. (0,1,2,3,4)
#define VOLTAGE0		0b0000		//voltages 0 - 3
#define VOLTAGE1		0b0001		//voltages 4 - 7
#define TEMP0			0b0010		//temps 0 - 3
#define TEMP1			0b0011		//temps 4 - 7
#define FLAGS07			0b0100		//flags 0 - 7
#define VOLTTEMP2		0b0101		//voltage and temperature 8 - 9
#define FLAGS89			0b0110		//flags 8 - 9

//switch statuses from steering wheel to array board
#define LIGHTS_MSG_ID	0X150
#define LIGHTS_MSG_DLC	1

#define DUMMYID			0b10000000000	//do not go above 11111110000


#endif	// __FLUXCAN_H__
