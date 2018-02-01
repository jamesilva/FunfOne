#include "effects.h"

int i;
#include <math.h>
#define BP_MAX_COEFS 50
#define PI 3.1415926

#define APP_RANGE_MIN 0
#define APP_RANGE_MAX 19

struct bp_filter s_wahfilter;

////////////////////////////////
//NOISE GATE
////////////////////////////////
short int PosGate, NegGate;
int Treshold, GateRelease, GateDepth;
int PosGateCounter, NegGateCounter, PosOffCounter, NegOffCounter;
char TurnOff;
char error;

////////////////////////////////
//RF
////////////////////////////////
#define RF_MAX_VALUE 2000
#define RF_MIN_VALUE 0

////////////////////////////////
//Distortion
////////////////////////////////
int gain;
short int upper_threshold, lower_threshold;
#define DISTORTION_GAIN_MIN 2500
#define DISTORTION_GAIN_MAX 4000

////////////////////////////////
//TREMOLO
////////////////////////////////
#define TREMOLO_DEPTH_MIN 1200
#define TREMOLO_DEPTH_MAX 4000
#define TREMOLO_RATE_MIN 30
#define TREMOLO_RATE_MAX 430
uint16_t nSineTable[1000] ={2048,2060,2073,2086,2099,2112,2125,2138,2150,2163,2176,2189,2202,2215,2227,2240,
2253,2266,2279,2291,2304,2317,2330,2342,2355,2368,2380,2393,2406,2419,2431,2444,
2456,2469,2482,2494,2507,2519,2532,2544,2557,2569,2582,2594,2606,2619,2631,2643,
2656,2668,2680,2692,2705,2717,2729,2741,2753,2765,2777,2789,2801,2813,2825,2837,
2849,2861,2872,2884,2896,2908,2919,2931,2942,2954,2966,2977,2988,3000,3011,3023,
3034,3045,3056,3068,3079,3090,3101,3112,3123,3134,3145,3155,3166,3177,3188,3198,
3209,3220,3230,3241,3251,3261,3272,3282,3292,3302,3313,3323,3333,3343,3353,3363,
3372,3382,3392,3402,3411,3421,3430,3440,3449,3458,3468,3477,3486,3495,3504,3513,
3522,3531,3540,3549,3558,3566,3575,3583,3592,3600,3609,3617,3625,3633,3641,3649,
3657,3665,3673,3681,3689,3696,3704,3711,3719,3726,3734,3741,3748,3755,3762,3769,
3776,3783,3790,3797,3803,3810,3816,3823,3829,3836,3842,3848,3854,3860,3866,3872,
3878,3883,3889,3895,3900,3906,3911,3916,3921,3927,3932,3937,3942,3946,3951,3956,
3961,3965,3970,3974,3978,3983,3987,3991,3995,3999,4003,4006,4010,4014,4017,4021,
4024,4027,4031,4034,4037,4040,4043,4046,4048,4051,4054,4056,4059,4061,4063,4066,
4068,4070,4072,4074,4075,4077,4079,4080,4082,4083,4085,4086,4087,4088,4089,4090,
4091,4092,4092,4093,4094,4094,4094,4095,4095,4095,4095,4095,4095,4095,4094,4094,
4094,4093,4092,4092,4091,4090,4089,4088,4087,4086,4085,4083,4082,4080,4079,4077,
4075,4074,4072,4070,4068,4066,4063,4061,4059,4056,4054,4051,4048,4046,4043,4040,
4037,4034,4031,4027,4024,4021,4017,4014,4010,4006,4003,3999,3995,3991,3987,3983,
3978,3974,3970,3965,3961,3956,3951,3946,3942,3937,3932,3927,3921,3916,3911,3906,
3900,3895,3889,3883,3878,3872,3866,3860,3854,3848,3842,3836,3829,3823,3816,3810,
3803,3797,3790,3783,3776,3769,3762,3755,3748,3741,3734,3726,3719,3711,3704,3696,
3689,3681,3673,3665,3657,3649,3641,3633,3625,3617,3609,3600,3592,3583,3575,3566,
3558,3549,3540,3531,3522,3513,3504,3495,3486,3477,3468,3458,3449,3440,3430,3421,
3411,3402,3392,3382,3372,3363,3353,3343,3333,3323,3313,3302,3292,3282,3272,3261,
3251,3241,3230,3220,3209,3198,3188,3177,3166,3155,3145,3134,3123,3112,3101,3090,
3079,3068,3056,3045,3034,3023,3011,3000,2988,2977,2966,2954,2942,2931,2919,2908,
2896,2884,2872,2861,2849,2837,2825,2813,2801,2789,2777,2765,2753,2741,2729,2717,
2705,2692,2680,2668,2656,2643,2631,2619,2606,2594,2582,2569,2557,2544,2532,2519,
2507,2494,2482,2469,2456,2444,2431,2419,2406,2393,2380,2368,2355,2342,2330,2317,
2304,2291,2279,2266,2253,2240,2227,2215,2202,2189,2176,2163,2150,2138,2125,2112,
2099,2086,2073,2060,2048,2035,2022,2009,1996,1983,1970,1957,1945,1932,1919,1906,
1893,1880,1868,1855,1842,1829,1816,1804,1791,1778,1765,1753,1740,1727,1715,1702,
1689,1676,1664,1651,1639,1626,1613,1601,1588,1576,1563,1551,1538,1526,1513,1501,
1489,1476,1464,1452,1439,1427,1415,1403,1390,1378,1366,1354,1342,1330,1318,1306,
1294,1282,1270,1258,1246,1234,1223,1211,1199,1187,1176,1164,1153,1141,1129,1118,
1107,1095,1084,1072,1061,1050,1039,1027,1016,1005,994,983,972,961,950,940,
929,918,907,897,886,875,865,854,844,834,823,813,803,793,782,772,
762,752,742,732,723,713,703,693,684,674,665,655,646,637,627,618,
609,600,591,582,573,564,555,546,537,529,520,512,503,495,486,478,
470,462,454,446,438,430,422,414,406,399,391,384,376,369,361,354,
347,340,333,326,319,312,305,298,292,285,279,272,266,259,253,247,
241,235,229,223,217,212,206,200,195,189,184,179,174,168,163,158,
153,149,144,139,134,130,125,121,117,112,108,104,100,96,92,89,
85,81,78,74,71,68,64,61,58,55,52,49,47,44,41,39,
36,34,32,29,27,25,23,21,20,18,16,15,13,12,10,9,
8,7,6,5,4,3,3,2,1,1,1,0,0,0,0,0,
0,0,1,1,1,2,3,3,4,5,6,7,8,9,10,12,
13,15,16,18,20,21,23,25,27,29,32,34,36,39,41,44,
47,49,52,55,58,61,64,68,71,74,78,81,85,89,92,96,
100,104,108,112,117,121,125,130,134,139,144,149,153,158,163,168,
174,179,184,189,195,200,206,212,217,223,229,235,241,247,253,259,
266,272,279,285,292,298,305,312,319,326,333,340,347,354,361,369,
376,384,391,399,406,414,422,430,438,446,454,462,470,478,486,495,
503,512,520,529,537,546,555,564,573,582,591,600,609,618,627,637,
646,655,665,674,684,693,703,713,723,732,742,752,762,772,782,793,
803,813,823,834,844,854,865,875,886,897,907,918,929,940,950,961,
972,983,994,1005,1016,1027,1039,1050,1061,1072,1084,1095,1107,1118,1129,1141,
1153,1164,1176,1187,1199,1211,1223,1234,1246,1258,1270,1282,1294,1306,1318,1330,
1342,1354,1366,1378,1390,1403,1415,1427,1439,1452,1464,1476,1489,1501,1513,1526,
1538,1551,1563,1576,1588,1601,1613,1626,1639,1651,1664,1676,1689,1702,1715,1727,
1740,1753,1765,1778,1791,1804,1816,1829,1842,1855,1868,1880,1893,1906,1919,1932,
1945,1957,1970,1983,1996,2009,2022,2035};

int no_samples = 1000;
int count, sample, LFO;
int depth, rate;

////////////////////////////////
//DELAY
////////////////////////////////
#define DELAY_TIME_MIN   4000
#define DELAY_TIME_MAX   18500
//#define DELAY_TIME_MAX   2
#define DELAY_FEEDBACK_MIN   5
#define DELAY_FEEDBACK_MAX   50
unsigned int DelayCounter = 0, Delay_Time = 9999;
float Delay_Feedback;
short int PosDelayBuffer[DELAY_TIME_MAX] = {0}; //, NegDelayBuffer[MAX_DELAY_DELAY] = {0};

/*
//REVERB
#define REVERB_TIME_MAX  10000
#define REVERB_TIME_MAX  100
short int superefeito = 1;
int ReverbCounter_Pos = 0, ReverbCounter_Neg = 0;
int EchoDepth = 1000, Reverb_Depth = 9999;
short int PosReverbBuffer[REVERB_TIME_MAX], NegReverbBuffer[REVERB_TIME_MAX];
*/

//OCTAVER
#define MAX_DELAY_OCTAVER 2000
short int PosOctaverBuffer[MAX_DELAY_OCTAVER], NegOctaverBuffer[MAX_DELAY_OCTAVER];
unsigned short int write_pt = 0;
unsigned int read_pt_A = 0, read_pt_B = 0; //MAX_DELAY_OCTAVER/2;
unsigned int divider = 0,	oct_volume, dir_volume;
unsigned short int octave = 0;

//////////////////////
//CHORUS
//////////////////////
#define CHORUS_DEPTH_MIN 15
#define CHORUS_DEPTH_MAX 100
#define CHORUS_RATE_MIN 1
#define CHORUS_RATE_MAX 25
#define MAX_DELAY_CHORUS  500
#define MIN_DELAY_CHORUS  100
unsigned short int PosChorusBuffer[MAX_DELAY_CHORUS], NegChorusBuffer[MAX_DELAY_CHORUS];
unsigned int Chorus_Depth = 50, ChorusCounter = 0, Chorus_Rate = 15, sample_chorus, LFO_chorus, count_chorus, volume_depth;

static int effect_value;
int volume = 4000, i;

extern QueueHandle_t xQueue_RF_Effect;

void Effects(void* args){
	
	Input_Enable();
	Output_Enable();
	
	//INIT FILTER
	//bp_iir_init(44100, 0.2, 1, 10, 500);
	
	for(;;){

		xQueueReceive(xQueue_RF_Effect, &effect_value, 0);
		xSemaphoreTake(xSemaphore_SignalAcquisition, portMAX_DELAY);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
		
		if(effect_value>RF_MAX_VALUE)
			effect_value=RF_MAX_VALUE;
		if(effect_value<RF_MIN_VALUE)
			effect_value=RF_MIN_VALUE;
		

	  PosBuf[0] = PosInBuf[0];
		NegBuf[0] = NegInBuf[0];
		
				//noise_gate();

	
		for(i=0;i<5;i++){
		switch(FXorder[i]){
			case NO_EFFECT: 
				break;
			case TREM: 
				tremolo_effect();
				break;
			case DISTORTION:
			//	GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
				distortion_effect();
				break;
			case CHORUS:
			//	GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
				chorus_effect();
				break;
			case REVERB:
				reverb_effect();
				break;
			case PITCH:
				break;
			case VOLUME:
				volume_effect();
				break;
			case WAH:
				wah_effect();
				break;
			case DELAY:
				delay_effect();
				break;
			case OCTAVER:
				octaver_effect();
				break;
			default:
				error=1;	
				break;
		}
	}
		//delay_effect();
		//volume_effect();
		//octaver_effect();
		//distortion_effect();
		//tremolo_effect();
		//reverb_effect();
		//chorus_effect();
		
		//wah_effect();
		
		//Bird_wahwah_effect();

		PosOutBuf[0] = map(PosBuf[0], 0, 4095, 1, volume);
		NegOutBuf[0] = map(NegBuf[0], 0, 4095, 1, volume);
		}
}



float map(float x, float in_min, float in_max, float out_min, float out_max){
	
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	
}

void noise_gate(){
	
	Treshold = 2000;
	GateDepth = 5000;
	GateRelease = 10000;
	
	PosGate = PosBuf[0];
	NegGate = NegBuf[0];
	
	if(TurnOff){
			PosGateCounter = 0;
			NegGateCounter = 0;
		if(PosGate > Treshold)
			PosOffCounter++;
		
		if(NegGate > Treshold)
			NegOffCounter++;
		
		if(PosOffCounter > GateRelease && NegOffCounter > GateRelease){
			PosOffCounter = 0;
			NegOffCounter = 0;
			TurnOff = 0;
		}
		
		PosBuf[0] = 0;
		NegBuf[0] = 0;	
	}
	
	else{	
			PosOffCounter = 0;
			NegOffCounter = 0;
		if(PosGate < Treshold)
			PosGateCounter++;
		
		if(NegGate < Treshold)
			NegGateCounter++;
		
		if(PosGateCounter > GateDepth && NegGateCounter > GateDepth){
			PosGateCounter = 0;
			NegGateCounter = 0;
			TurnOff = 1;
		}	
	}
}


void volume_effect(void){
	
	if(s_FXvolume.volume_efeito){
		s_FXvolume.value = map(effect_value, RF_MAX_VALUE, RF_MIN_VALUE, 50, 4095);
		//s_FXvolume.value = effect_value; //efeito;
	}
		
	PosBuf[0] = map(PosBuf[0], 0, 4095, 1, s_FXvolume.value);
	NegBuf[0] = map(NegBuf[0], 0, 4095, 1, s_FXvolume.value);
	
}

void distortion_effect(void){
	
	if(s_FXdist.gain==19) s_FXdist.gain = 18;
	
	gain = map(s_FXdist.gain, APP_RANGE_MIN, APP_RANGE_MAX, DISTORTION_GAIN_MIN, DISTORTION_GAIN_MAX);
	
	upper_threshold = map(gain, 1000, 4000, 4000, 2000);
  lower_threshold = map(gain, 1000, 4000, 0000, 2000);
	
	if(PosBuf[0] >= upper_threshold) PosBuf[0] = upper_threshold;
	else if(PosBuf[0] < lower_threshold) PosBuf[0] = lower_threshold;

	if(NegBuf[0] >= upper_threshold) NegBuf[0] = upper_threshold;
	else if(NegBuf[0] < lower_threshold) NegBuf[0] = lower_threshold;

		//adjust the volume with POT2 with constant loudness
	PosBuf[0] = map(PosBuf[0], lower_threshold, upper_threshold, 201, 3800);
	NegBuf[0] = map(NegBuf[0], lower_threshold, upper_threshold, 201, 3800);
		
}


void tremolo_effect(void){
	
	if(s_FXtrem.tremolo_rate_efeito)
		rate = map(effect_value, RF_MAX_VALUE, RF_MIN_VALUE, TREMOLO_RATE_MIN, TREMOLO_RATE_MAX);
	
	else
		rate = map(s_FXtrem.rate, APP_RANGE_MIN, APP_RANGE_MAX, TREMOLO_RATE_MIN, TREMOLO_RATE_MAX);
	
	depth = map(s_FXtrem.depth, APP_RANGE_MIN, APP_RANGE_MAX, TREMOLO_DEPTH_MIN, TREMOLO_DEPTH_MAX);
	
	count++; 
	if (count>=2000){ //160 chosen empirically
		count=0;
		sample = sample + rate;
		if(sample >= no_samples){
			sample=0;
		}
	}
	
	//Create the Low Frequency Oscillator signal with depth control based in POT1.
	LFO = map(nSineTable[sample], 0, 4095, (4095 - depth), 4095);	
		
		//Modulate the output signals based on the sinetable.
	PosBuf[0] = map(PosBuf[0], 1, 4095, 1, LFO);
	NegBuf[0] = map(NegBuf[0], 1, 4095, 1, LFO);

		//Add volume feature with POT2
	PosBuf[0] = map(PosBuf[0], 1, 4095, 1, 4000);
	NegBuf[0] = map(NegBuf[0], 1, 4095, 1, 4000);

}


void delay_effect(void){
	
	if(s_FXdelay.delay_time_efeito)
		Delay_Time = map(effect_value, RF_MAX_VALUE, RF_MIN_VALUE, DELAY_TIME_MIN, DELAY_TIME_MAX);
	
	else
		Delay_Time = map(s_FXdelay.time, APP_RANGE_MIN, APP_RANGE_MAX, DELAY_TIME_MIN, DELAY_TIME_MAX);
	
	Delay_Feedback = map(s_FXdelay.feedback, APP_RANGE_MIN, APP_RANGE_MAX, DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX);
	
	Delay_Feedback = Delay_Feedback / 100;
	
	PosDelayBuffer[DelayCounter] = (PosBuf[0] + PosDelayBuffer[DelayCounter])*Delay_Feedback;
	
	PosDelayBuffer[DelayCounter] = map(PosDelayBuffer[DelayCounter], 0, 2600, 1, 4095);

	DelayCounter++;

	//If we reach the end of the buffer, we reset the counter
	if(DelayCounter >= Delay_Time) {
		DelayCounter = 0;
	}
	
	PosBuf[0]=map(PosBuf[0] + PosDelayBuffer[DelayCounter], 0, 8190, 0, 4095);

}


void wah_effect(void){
	
	bp_iir_setup(&s_wahfilter, 20);
	PosBuf[0] = (unsigned short int)bp_iir_filter((double)PosBuf[0], &s_wahfilter);
	NegBuf[0] = (unsigned short int)bp_iir_filter((double)NegBuf[0], &s_wahfilter); 

}


void reverb_effect(void){
	
/*	 //Store current readings in ECHO mode
	PosReverbBuffer[ReverbCounter_Pos] = (PosBuf[0] + (PosReverbBuffer[ReverbCounter_Pos]))>>1;
	NegReverbBuffer[ReverbCounter_Neg] = (NegBuf[0] + (NegReverbBuffer[ReverbCounter_Neg]))>>1; 
	
	//Adjust Delay Depth based in POT0 and POT1 position.
	//EchoDepth = map(EchoDepth>>3 , 0, 512, 1, MAX_DELAY_REVERB);
	//Reverb_Depth = map(Reverb_Depth>>3, 0, 512, 1, MAX_DELAY_REVERB);
 
	//Increse/reset delay counter.   
	ReverbCounter_Pos++;
	ReverbCounter_Neg++;
	
	if(ReverbCounter_Pos >= EchoDepth) ReverbCounter_Pos = 0; 
	if(ReverbCounter_Neg >= Reverb_Depth) ReverbCounter_Neg = 0; 
 
	//Calculate the output as the sum of DelayBuffer_A + DelayBuffer_B 
	//PosBuf[0] = (PosReverbBuffer[DelayCounter_Pos]);
	//NegBuf[0] = (NegReverbBuffer[DelayCounter_Neg]);
	
	//PosBuf[0] = (PosReverbBuffer[ReverbCounter_Pos]);
	//NegBuf[0] = (NegReverbBuffer[ReverbCounter_Neg]);
	
	PosBuf[0]=map(PosBuf[0] + (PosReverbBuffer[ReverbCounter_Pos]), 0, 8190, 0, 4095);
	NegBuf[0]=map(NegBuf[0] + (NegReverbBuffer[ReverbCounter_Neg]), 0, 8190, 0, 4095);*/
 
}


void octaver_effect(void){
	
	PosOctaverBuffer[write_pt] = PosBuf[0];
	NegOctaverBuffer[write_pt] = NegBuf[0];
 
	//Increse/reset delay counter.
	write_pt++;
	if(write_pt >= MAX_DELAY_OCTAVER) write_pt = 0; 

	octave = s_FXoctaver.octave;
	dir_volume = map(s_FXoctaver.dir_volume, APP_RANGE_MIN, APP_RANGE_MAX, 1, 4000);
	oct_volume = map(s_FXoctaver.oct_volume, APP_RANGE_MIN, APP_RANGE_MAX, 1, 4000);
	
	PosOctaverBuffer[read_pt_A] = map(PosOctaverBuffer[read_pt_A], 0, 4095, 1, oct_volume);
	NegOctaverBuffer[read_pt_B] = map(NegOctaverBuffer[read_pt_B], 0, 4095, 1, oct_volume);
	
	PosBuf[0] = map(PosBuf[0], 0, 4095, 1, dir_volume);
	NegBuf[0] = map(NegBuf[0], 0, 4095, 1, dir_volume);
	
	PosBuf[0] = map(PosOctaverBuffer[read_pt_A] + PosBuf[0], 0, 8190, 1, 4095);
	NegBuf[0] = map(NegOctaverBuffer[read_pt_B] + NegBuf[0], 0, 8190, 1, 4095);
 
	if (octave){ 
		read_pt_A = read_pt_A + 2;
		read_pt_B = read_pt_B + 2;
	}
	else {
	 divider++;
	 if (divider>=2){
			read_pt_A = read_pt_A + 1;
			read_pt_B = read_pt_B + 1;
			divider=0;
		}
	}
 
	if(read_pt_A >= MAX_DELAY_OCTAVER) read_pt_A = 0; 
	if(read_pt_B >= MAX_DELAY_OCTAVER) read_pt_B = 0; 

}


void chorus_effect(void){

	volume_depth = 4000;
	
	if(s_FXchorus.chorus_rate_efeito)
		Chorus_Rate = map(effect_value, RF_MAX_VALUE, RF_MIN_VALUE, CHORUS_RATE_MIN, CHORUS_RATE_MAX);
	
	else
		Chorus_Rate = map(s_FXchorus.rate, APP_RANGE_MIN, APP_RANGE_MAX, CHORUS_RATE_MIN, CHORUS_RATE_MAX);

	Chorus_Depth = map(s_FXchorus.depth, APP_RANGE_MIN, APP_RANGE_MAX, CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX);
	
	//Store current readings  
  PosChorusBuffer[ChorusCounter] = PosBuf[0];
	NegChorusBuffer[ChorusCounter] = NegBuf[0];
	
	ChorusCounter++;
  if(ChorusCounter >= Chorus_Depth){
    ChorusCounter = 0;
	}
	
	count_chorus++; 
	if (count_chorus>=200){ //160 chosen empirically
		count_chorus=0;
		sample_chorus = sample_chorus + Chorus_Rate;
		if(sample_chorus >= no_samples){
			sample_chorus=0;
		}
	}
	
	//Create the Low Frequency Oscillator signal with depth control based in POT1.
	LFO_chorus = map(nSineTable[sample_chorus], 0, 4095, (4095 - volume_depth), 4095);	
	
	PosChorusBuffer[ChorusCounter] = map(PosChorusBuffer[ChorusCounter], 1, 4095, 1, LFO_chorus);
	NegChorusBuffer[ChorusCounter] = map(NegChorusBuffer[ChorusCounter], 1, 4095, 1, LFO_chorus);

	PosBuf[0]=map(PosBuf[0] + (PosChorusBuffer[ChorusCounter]), 0, 8190, 0, 4095);
	NegBuf[0]=map(NegBuf[0] + (NegChorusBuffer[ChorusCounter]), 0, 8190, 0, 4095);

}

static struct bp_coeffs bp_coeff_arr[BP_MAX_COEFS];

/*This initialization function will create the
band pass filter coefficients array, you have to specify:
fsfilt = Sampling Frequency
gb     = Gain at cut frequencies (0 to 1)
Q      = Q factor, Higher Q gives narrower band
fstep  = Frequency step to increase center frequencies
in the array
fmin   = Minimum frequency for the range of center   frequencies
*/
void bp_iir_init(double fsfilt,double gb,double Q,short fstep, short fmin) {
      int i;
      double damp;
      double wo;
      
       damp = gb/sqrt(1 - pow(gb,2));

	for (i=0;i<BP_MAX_COEFS;i++) {
    wo = 2*PI*(fstep*i + fmin)/fsfilt;
		bp_coeff_arr[i].e = 1/(1 + damp*tan(wo/(Q*2)));
		bp_coeff_arr[i].p = cos(wo);
		bp_coeff_arr[i].d[0] = (1-bp_coeff_arr[i].e);
		bp_coeff_arr[i].d[1] = 2*bp_coeff_arr[i].e*bp_coeff_arr[i].p;
		bp_coeff_arr[i].d[2] = (2*bp_coeff_arr[i].e-1);
	}
}

/*This function loads a given set of band pass filter coefficients acording to a center frequency index
into a band pass filter object
H = filter object
ind = index of the array mapped to a center frequency
*/
void bp_iir_setup(struct bp_filter * H,int ind) {
	H->e = bp_coeff_arr[ind].e;
	H->p = bp_coeff_arr[ind].p;
	H->d[0] = bp_coeff_arr[ind].d[0];
	H->d[1] = bp_coeff_arr[ind].d[1];
	H->d[2] = bp_coeff_arr[ind].d[2];
}

/*This function loads a given set of band pass filter coefficients acording to a center frequency index
into a band pass filter object
H = filter object
ind = index of the array mapped to a center frequency
*/
double bp_iir_filter(double yin, struct bp_filter * H) {
	double yout;

	H->x[0] =  H->x[1]; 
	H->x[1] =  H->x[2]; 
	H->x[2] = yin;
	
	H->y[0] =  H->y[1]; 
	H->y[1] =  H->y[2]; 

	H->y[2] = H->d[0]* H->x[2] - H->d[0]* H->x[0] + (H->d[1]* H->y[1]) - H->d[2]* H->y[0];
	
	yout =  H->y[2];

	return yout;
}
