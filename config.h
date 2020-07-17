/* CONFIGURATION FILE */

#define MUX0 14
#define MUX1 15
#define MUX2 12
#define MUX3 9
#define MUX4 13
#define MUX5 11
#define MUX6 10

#define MUXDswitch1_A 5
#define MUXDswitch1_B 12
#define MUXDswitch2_A 14
#define MUXDswitch2_B 15
#define MUXDswitch3_A 0
#define MUXDswitch3_B 8
#define MUXDswitch4_A 6
#define MUXDswitch4_B 7

//Aux Pins
#define led1 10
#define led2 11
#define PINarcade 13

//Mux Pins
#define S0 5
#define S1 4
#define S2 3
#define S3 2
#define MZ A8
#define DZ 1
#define MnumControls 16


//CC
#define CCinGain 96
#define CCfeedbackGain 97
#define CClowpassFreq 98
#define CChighpassFreq 99

#define CCoutVol 100
#define CCwsWarp 101
#define CCwaveformSelect1  102
#define CCwaveformSelect2  103
#define CCwsDrive 104
#define CCbcRate 105
#define CCbcBits 106
#define CCdriveWet 107

#define CCglitchSize 110
#define CCglitchJitter 111
#define CCglitchHead1 112
#define CCglitchHead2 113
#define CCglitchHead3 114
#define CCglitchHead4 115
#define CCglitchWet 116

float ws_warp = 0;
float ws_drive = 1;
float ws_arcade = 0;

int waveform_select1 = 1;
int waveform_select2 = 2;

float L_glitch_head2 = 0.3;
float L_glitch_head3 = 0.3;

float R_glitch_head2 = 0.3;
float R_glitch_head3 = 0.3;


//rar constants
const int muxTimeMin = 1000; // minimum micro-seconds between MUX reads
const int channel = 1; // MIDI channel
const int MUX_PINS = 16; // number of MUX Channnels
//rar variables
// a data array and a lagged copy to tell when MIDI changes are required
byte data[MUX_PINS];
byte dataLag[MUX_PINS]; // when lag and new are not the same then update MIDI CC value
byte i=0; // global index for MUX channel reads
//timer variables
elapsedMicros mux1Updated; // switch to micros to run at speed and tune with muxTimeMin setting above
//rar initialization
ResponsiveAnalogRead *analog[MUX_PINS];
//digital mux bounce setup
const int bounceTime = 5; // 5 ms is usually sufficient
Bounce *digital[MUX_PINS];
//arcade button  bounce setup
Bounce arcade = Bounce(PINarcade, bounceTime);


//------------------LFO-------------------- TO-DO CC//
unsigned int LFOspeed = 7000;
float LFOdepth = 0.2;
float LFOrangeTop = 1;
