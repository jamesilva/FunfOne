#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

//**************************************************************************************

#include <stm32f4xx.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//**************************************************************************************

#define BLUETOOTH_TX GPIO_Pin_10
#define BLUETOOTH_RX GPIO_Pin_11
#define MAX_PRESET_SIZE 39
#define TERM_C 75
#define ADD_EFFECT_C 35
#define REM_EFFECT_C 36
#define PRESET_C 80
#define NO_EFFECT   48
#define TREM				49
#define DISTORTION	50
#define CHORUS 			51
#define REVERB		  52
#define PITCH 			53
#define VOLUME 			54
#define WAH 				55 
#define DELAY			  56
#define OCTAVER 		57
	
//**************************************************************************************

struct s_trem{
	unsigned int depth;
	unsigned int rate;
	char tremolo_rate_efeito;
}extern s_FXtrem;

struct s_dist{
	unsigned int gain;
}extern s_FXdist;

struct s_delay{
	unsigned int time;
	unsigned int feedback;
	char delay_time_efeito;
}extern s_FXdelay;

struct s_chorus{
	unsigned int depth;
	unsigned int rate;
	char chorus_rate_efeito;
}extern s_FXchorus;

struct s_reverb{
	unsigned int decay;
	unsigned int density;
}extern s_FXreverb;

struct s_pitch{
	unsigned int pitch;
	char pitch_shift_efeito;
}extern s_FXpitch;

struct s_volume{
	unsigned int value;
	char volume_efeito;
}extern s_FXvolume;

struct s_wah{
	unsigned int value;
	char wah_efeito;
}extern s_FXwah;

struct s_octaver{
	unsigned int octave;
	unsigned int dir_volume;
	unsigned int oct_volume;
}extern s_FXoctaver;

extern char FXorder[5];
extern char preset[5][MAX_PRESET_SIZE];

extern SemaphoreHandle_t 	xSemaphore_BluetoothReceive;
extern char Command[MAX_PRESET_SIZE];
extern char PresetNum;
extern char error;

//**************************************************************************************

void Bluetooth_Config(void);
void BluetoothReceive(void*);

//**************************************************************************************

#endif /*_BLUETOOTH_H_ */
