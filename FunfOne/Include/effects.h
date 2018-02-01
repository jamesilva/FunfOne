#ifndef	_EFFECTS_H_
#define _EFFECTS_H_

#include "conversion.h"
#include "bluetooth.h"
#include "rf.h"

extern short int PosInBuf[BUFFERSIZE];
extern short int NegInBuf[BUFFERSIZE];

//Processing Buffers//
extern short int PosBuf[BUFFERSIZE];
extern short int NegBuf[BUFFERSIZE];

//Output Buffers, source from the DACs//
extern short int PosOutBuf[BUFFERSIZE];
extern short int NegOutBuf[BUFFERSIZE];

struct bp_coeffs{
	double e;
	double p;
	double d[3];
};

struct bp_filter{
	double e;
	double p;
	double d[3];
	double x[3];
	double y[3];
}extern s_wahfilter;

extern void bp_iir_init(double fsfilt,double gb,double Q,short fstep, short fmin);
extern void bp_iir_setup(struct bp_filter * H,int index);
extern double bp_iir_filter(double yin,struct bp_filter * H);


float map(float x, float in_min, float in_max, float out_min, float out_max);
void noise_gate(void);
void distortion_effect(void);
void tremolo_effect(void);
void volume_effect(void);
void delay_effect(void);
void wah_effect(void);
void reverb_effect(void);
void octaver_effect(void);
void chorus_effect(void);
void Bird_wahwah_effect(void);

void Effects(void*);
#endif /*_EFFECTS_H_*/
