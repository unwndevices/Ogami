#include "config.h"
#include <Audio.h>
#include "waveforms.h"
#include "effect_glitchdelay.h"
#include "effect_dynamics.h"
#include "Util.h"
#include <ResponsiveAnalogRead.h>
#include <Bounce.h>


AudioInputI2S in;                        //xy=136,486
AudioMixer4 feedback_mixer;              //xy=362,498
AudioFilterStateVariable lowpass;       //xy=602,497
AudioFilterStateVariable highpass; //xy=752,498
AudioFilterStateVariable highpassF; //xy=915,795
AudioEffectWaveshaper waveshaper;        //xy=917,486
AudioFilterStateVariable lowpassF;       //xy=922,911
AudioEffectBitcrusher bitcrusher;        //xy=1109,486
AudioMixer4 drive_mixer;                 //xy=1330,504
AudioEffectDynamics compressor;          //xy=1537,503
AudioEffectGlitch glitch;                //xy=1609,737
AudioMixer4 L_glitch_mixer;              //xy=1886,347
AudioMixer4 R_glitch_mixer;              //xy=1905,909
AudioMixer4 R_dry_wet_mixer;              //xy=2213,667
AudioMixer4 L_dry_wet_mixer;              //xy=2214,358
AudioAmplifier L_gain;                   //xy=2424,356
AudioAmplifier R_gain;                   //xy=2430,670
AudioOutputI2S out;                      //xy=2792,520
AudioConnection patchCord1(in, 0, feedback_mixer, 0);
AudioConnection patchCord2(feedback_mixer, 0, lowpass, 0);
AudioConnection patchCord3(lowpass, 0, highpass, 0);
AudioConnection patchCord4(highpass, 2, waveshaper, 0);
AudioConnection patchCord5(highpassF, 2, feedback_mixer, 1);
AudioConnection patchCord6(waveshaper, bitcrusher);
AudioConnection patchCord7(lowpassF, 0, highpassF, 0);
AudioConnection patchCord8(bitcrusher, 0, drive_mixer, 0);
AudioConnection patchCord9(drive_mixer, compressor);
AudioConnection patchCord10(compressor, glitch);
AudioConnection patchCord11(compressor, 0, L_dry_wet_mixer, 0);
AudioConnection patchCord12(compressor, 0, R_dry_wet_mixer, 0);
AudioConnection patchCord13(glitch, 0, L_glitch_mixer, 0);
AudioConnection patchCord14(glitch, 0, R_glitch_mixer, 0);
AudioConnection patchCord15(glitch, 0, lowpassF, 0);
AudioConnection patchCord16(glitch, 1, L_glitch_mixer, 1);
AudioConnection patchCord17(glitch, 1, R_glitch_mixer, 1);
AudioConnection patchCord18(glitch, 2, L_glitch_mixer, 2);
AudioConnection patchCord19(glitch, 2, R_glitch_mixer, 2);
AudioConnection patchCord20(glitch, 3, L_glitch_mixer, 3);
AudioConnection patchCord21(glitch, 3, R_glitch_mixer, 3);
AudioConnection patchCord22(L_glitch_mixer, 0, L_dry_wet_mixer, 1);
AudioConnection patchCord23(R_glitch_mixer, 0, R_dry_wet_mixer, 1);
AudioConnection patchCord24(R_dry_wet_mixer, R_gain);
AudioConnection patchCord25(L_dry_wet_mixer, L_gain);
AudioConnection patchCord26(L_gain, 0, out, 0);
AudioConnection patchCord27(R_gain, 0, out, 1);
AudioControlSGTL5000 sgtl5000;           //xy=672,1106


//setup parameters
float outVol = 0.8;

//feedback_mixer
float dry_gain = 0.8;
float feedback_gain = 0.2;

//filters
float lowpass_freq = 15000;
float highpass_freq = 500;
//waveshaper parameters
float ws_warp = 0;
float ws_drive = 1;
float ws_arcade = 0;

//waveformsTable size
int waveform_select1 = 1;
int waveform_select2 = 2;

//bitcrusher parameters
int bc_rate = 1;
int bc_bits = 16;

float drive_wet = 0.6;
//glitch delay
float glitch_size = 0.3;
float glitch_jitter = 0.0;
float L_glitch_head1 = 0.4;
float L_glitch_head2 = 0.3;
float L_glitch_head3 = 0.3;
float L_glitch_head4 = 0;

float R_glitch_head1 = 0.4;
float R_glitch_head2 = 0.3;
float R_glitch_head3 = 0.3;
float R_glitch_head4 = 0;

float glitch_wet = 0;

//shimmer
float reverb_size = 0.4;
float reverb_tone = 0.4;
const int granular_memory_size = 12800;
int16_t L_granular_up_memory[granular_memory_size];
int16_t L_granular_down_memory[granular_memory_size];
int16_t R_granular_up_memory[granular_memory_size];
int16_t R_granular_down_memory[granular_memory_size];
float up_speed = 2;
float down_speed = 0.5;
int grain_size = 54;
float reverb_wet = 0;
float shimmer_wet = 0;

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

void setup() {
	// patchA1.disconnect(); //to disconnect patch cables
	AudioMemory(32);

	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(MZ, INPUT);
	pinMode(DZ, INPUT);
	pinMode(PINarcade, INPUT);

	setupAnalog();
	setupDigital();
//MIDI SETUP
	usbMIDI.setHandleControlChange(myControlChange);
	// usbMIDI.setHandleClock(myClock); //MIDI SYNC
	// usbMIDI.setHandleStart(myStart);
	// usbMIDI.setHandleContinue(myContinue);
	// usbMIDI.setHandleStop(myStop);


	sgtl5000.enable();
	sgtl5000.volume(outVol);
	pinMode(led1, OUTPUT);

	//bitcrusher setup
	bitcrusher.sampleRate(44100 / bc_rate);
	bitcrusher.bits(bc_bits);

	//waveshaper setup
	waveshaper.shape(waveformsTable[0], 65);

	drive_mixer.gain(0, 1 - drive_wet);
	drive_mixer.gain(1, drive_wet);

	//dynamics setup
	//limit(threshold = -6.0f, attack = 0.005f, release = 0.01f)
	compressor.limit(-2.0f, 0.005f, 0.01f);
	//compression(threshold = -35.0f, attack = 0.005f, release = 0.2f, ratio = 45.0f, kneeWidth = 6.0f
	compressor.compression(-12.0f, 0.05f, 0.2f, 5.0f, 7.0f);
	compressor.autoMakeupGain(6.0f);

	//feedback mixer setup
	feedback_mixer.gain(0, dry_gain);
	feedback_mixer.gain(1, feedback_gain);

	//filters setup
	lowpass.frequency(lowpass_freq);
	lowpass.resonance(0.75);
	lowpass.octaveControl(0);
	lowpassF.frequency(lowpass_freq);
	lowpassF.resonance(0.7);
	lowpassF.octaveControl(0);
	highpass.frequency(highpass_freq);
	highpass.resonance(0.7);
	highpass.octaveControl(0);
	highpassF.frequency(300);
	highpassF.resonance(0.7);
	highpassF.octaveControl(0);

	//heads mixer setup
	L_glitch_mixer.gain(0, L_glitch_head1);
	L_glitch_mixer.gain(1, L_glitch_head2);
	L_glitch_mixer.gain(2, L_glitch_head3);
	L_glitch_mixer.gain(3, L_glitch_head4);
	R_glitch_mixer.gain(0, R_glitch_head1);
	R_glitch_mixer.gain(1, R_glitch_head2);
	R_glitch_mixer.gain(2, R_glitch_head3);
	R_glitch_mixer.gain(3, R_glitch_head4);

	//delay mixer setup
	L_dry_wet_mixer.gain(0, 1 - glitch_wet);
	L_dry_wet_mixer.gain(1, glitch_wet);
	R_dry_wet_mixer.gain(0, 1 - glitch_wet);
	R_dry_wet_mixer.gain(1, glitch_wet);

	L_gain.gain(1.2f);
	R_gain.gain(1.2f);

	//glitch jitter & size
	glitch.set_loop_size( 0, glitch_size );
	glitch.set_loop_size( 1, glitch_size );
	glitch.set_loop_size( 2, glitch_size );
	glitch.set_loop_size( 3, glitch_size );
	glitch.set_jitter( 0, glitch_jitter );
	glitch.set_jitter( 1, glitch_jitter );
	glitch.set_jitter( 2, glitch_jitter );
	glitch.set_jitter( 3, glitch_jitter );
	glitch.set_loop_moving(false);

}

void setupAnalog() {
	for (int i = 0; i < MUX_PINS; i++) {
		analog[i] = new ResponsiveAnalogRead(MZ, true); // initialize
	}
}

void setupDigital() {
	for (int i = 0; i < MUX_PINS; i++) {
		digital[i] = new Bounce(DZ, bounceTime); // initialize
	}
}

void loop() {
	LFOupdate(LFOrangeTop, LFOdepth);
	nextMUXpin();
	while (usbMIDI.read()) {
		// controllers must call .read() to keep the queue clear even if they are not responding to MIDI
	}
}


void myControlChange(byte channel, byte control, byte value) {
	switch (control) {
	case CCdry_gain:      //Output gain
		dry_gain = map((float)value, 0, 127, 0, 1);
		feedback_mixer.gain(0, dry_gain);
		break;

	case CCfeedback_gain:      //Output gain
		feedback_gain = map((float)value, 0, 127, 0, 0.5);
		feedback_mixer.gain(1, feedback_gain);
		break;

	case CClowpass_freq:      //Output gain
		lowpass_freq = map((float)value, 0, 127, 30, 15000);
		lowpass1.frequency(lowpass_freq);
		break;

	case CChighpass_freq:      //Highpass
		highpass_freq = map((float)value, 0, 127, 30, 15000);
		highpass1.frequency(highpass_freq);
		break;


	case CCoutVol:      //Output gain
		outVol = map((float)value, 0, 127, 0, 1);
		sgtl5000.volume(outVol);
		break;

	//Waveshaper CC
	case CCws_warp:      //waveshape Amount
		ws_warp = map((float)value, 0, 127, 0, 1);
		wsMorph();
		waveshaper.shape(newWaveform, 65);
		break;

	case CCws_drive:  //waveshape Amount
		ws_drive = map((float)value, 0, 127, 1, 4);
		wsMorph();
		waveshaper.shape(newWaveform, 65);
		break;

	//Bitcrusher CC
	case CCbc_rate:
		bc_rate = map ((int)value, 0, 127, 1, 64);
		bitcrusher.sampleRate(44100 / bc_rate);
		break;

	case CCbc_bits:
		bc_bits = map ((int)value, 0, 127, 1, 16);
		bitcrusher.bits(bc_bits);
		break;

	case CCdrive_wet:
		drive_wet = map((float)value, 0, 127, 0, 1);
		drive_mixer.gain(0, drive_wet);
		drive_mixer.gain(1, 1 - drive_wet);
		break;

	case CCglitch_size:
		glitch_size = map((float)value, 0, 127, 0, 1);
		glitch.set_loop_size( 0, glitch_size);
		glitch.set_loop_size( 1, glitch_size);
		glitch.set_loop_size( 2, glitch_size);
		glitch.set_loop_size( 3, glitch_size);
		break;

	case CCglitch_jitter:
		glitch_jitter = map((float)value, 0, 127, 0, 1);
		glitch.set_jitter( 0, glitch_jitter);
		glitch.set_jitter( 1, glitch_jitter);
		glitch.set_jitter( 2, glitch_jitter);
		glitch.set_jitter( 3, glitch_jitter);
		break;

	case CCglitch_head1:
		L_glitch_head1 = map((float)value, 0, 127, 0, 1);
		R_glitch_head1 = map((float)value, 0, 127, 0, 1);
		break;

	case CCglitch_head2:
		L_glitch_head2 = map((float)value, 0, 127, 0, 1);
		R_glitch_head2 = map((float)value, 0, 127, 0, 1);
		break;

	case CCglitch_head3:
		L_glitch_head3 = map((float)value, 0, 127, 0, 1);
		R_glitch_head3 = map((float)value, 0, 127, 0, 1);
		break;

	case CCglitch_head4:
		L_glitch_head4 = map((float)value, 0, 127, 0, 1);
		R_glitch_head4 = map((float)value, 0, 127, 0, 1);
		break;

	case CCglitch_wet:
		glitch_wet = map((float)value, 0, 127, 0, 1);
		L_dry_wet_mixer.gain(0, 1 - glitch_wet);
		L_dry_wet_mixer.gain(1, glitch_wet);
		break;

	case CCwaveform_select1:     //waveshape Select
		switch (value) {
		case 0:
			waveform_select1 = 0;
			break;
		case 1:
			waveform_select1 = 1;
			break;
		case 2:
			waveform_select1 = 2;
			break;
		}
	case CCwaveform_select2:     //waveshape Select
		switch (value) {
		case 0:
			waveform_select2 = 0;
			break;
		case 1:
			waveform_select2 = 1;
			break;
		case 2:
			waveform_select2 = 2;
			break;
		}

	}
}

//******ANALOG MULTIPLEXING CODE******//
void nextMUXpin(){
	if (mux1Updated>muxTimeMin) {
		// update the ResponsiveAnalogRead object every loop
		analog[i]->update();
		// if the repsonsive value has change, print out 'changed'
		if(analog[i]->hasChanged()) {
			data[i] = analog[i]->getValue()>>3;
			if (data[i] != dataLag[i]) {
				dataLag[i] = data[i];
				switch (i)
				{
				case MUXdry_gain:
					dry_gain = ((float)data[i], 0, 127, 0, 1.6);
					feedback_mixer.gain(0, dry_gain);
					break;

				case MUXfeedback_gain:
					feedback_gain = map((float)data[i], 0, 127, 0, 0.53);
					feedback_mixer.gain(1, feedback_gain);
					break;

				case MUXws_drive:
					L_glitch_head3 = map((float)data[i], 0, 127, 0, 1);
					R_glitch_head3 = map((float)data[i], 0, 127, 0, 1);
					LFOupdate(LFOrangeTop, LFOdepth);
					break;

				case MUXglitch_size:
					glitch_size = map((float)data[i], 0, 127, 0, 1);
					glitch.set_loop_size( 0, glitch_size );
					glitch.set_loop_size( 1, glitch_size );
					glitch.set_loop_size( 2, glitch_size );
					glitch.set_loop_size( 3, glitch_size );
					break;

				case MUXglitch_main:
					L_glitch_head2 = map((float)data[i], 0, 127, 0, 1);
					R_glitch_head2 = map((float)data[i], 0, 127, 0, 1);
					L_glitch_mixer.gain (0, L_glitch_head2);
					R_glitch_mixer.gain (0, R_glitch_head2);
					break;

				case MUXglitch_extra:
					L_glitch_head1 = map((float)data[i], 0, 127, 0, 1);
					R_glitch_head1 = map((float)data[i], 0, 127, 0, 1);
					LFOupdate(LFOrangeTop, LFOdepth);
					break;

				case MUXglitch_wet:
					glitch_wet = map((float)data[i], 0, 127, 0, 1);
					L_dry_wet_mixer.gain(0, 1 - glitch_wet);
					L_dry_wet_mixer.gain(1, glitch_wet);
					break;
				}
			}
		}
		//getDigitalData();
		arcade.update();
		if (arcade.risingEdge()) {
			glitch.set_freeze_active(true);
			digitalWrite(led1, HIGH);
		}
		if (arcade.fallingEdge()) {
			glitch.set_freeze_active(false);
			digitalWrite(led1, LOW);
		}
		//reset timer
		mux1Updated = 0;
		//increment index
		i++;
		if (i>15)   {i=0;}
		// set mux control pins for next pass
		digitalWrite(S0, HIGH && (i & B00000001));
		digitalWrite(S1, HIGH && (i & B00000010));
		digitalWrite(S2, HIGH && (i & B00000100));
		digitalWrite(S3, HIGH && (i & B00001000));
	}
}

//******DIGITAL MULTIPLEXING CODE******//
void getDigitalData(){
	for (int i=0; i<MUX_PINS; i++) {
		digital[i]->update();
		if (digital[MUXDswitch1_A]->risingEdge()) {
			Serial.println("switch 1 up");
			// ws_warp = 0;
			// wsMorph();
			// waveshaper.shape(newWaveform, 65);
		}
		else if (digital[MUXDswitch1_B]->risingEdge()) {
			Serial.println("switch 1 down");
			// ws_warp = 0.5;
			// wsMorph();
			// waveshaper.shape(newWaveform, 65);
		}
		else {
			// ws_warp = 1;
			// wsMorph();
			// waveshaper.shape(newWaveform, 65);
		}

		if (digital[MUXDswitch2_A]->risingEdge()) {
			Serial.println("switch 2 up");
		}
		else if (digital[MUXDswitch2_B]->risingEdge()) {
			Serial.println("switch 2 down");
		}
		else {}

		if (digital[MUXDswitch3_A]->risingEdge()) {
			Serial.println("switch 3 up");
			// glitch.set_jitter( 0, 0);
			// glitch.set_jitter( 1, 0);
			// glitch.set_jitter( 2, 0);
			// glitch.set_jitter( 3, 0);
		}
		else if (digital[MUXDswitch3_B]->risingEdge()) {
			Serial.println("switch 3 down");
			// glitch.set_jitter( 0, 1);
			// glitch.set_jitter( 1, 1);
			// glitch.set_jitter( 2, 1);
			// glitch.set_jitter( 3, 1);
		}
		else {
			// glitch.set_jitter( 0, 0.5);
			// glitch.set_jitter( 1, 0.5);
			// glitch.set_jitter( 2, 0.5);
			// glitch.set_jitter( 3, 0.5);
		}

		if (digital[MUXDswitch4_A]->risingEdge()) {
			Serial.println("switch 4 up");
		}
		else if (digital[MUXDswitch4_B]->risingEdge()) {
			Serial.println("switch 4 down");
		}
		else {}
	}
}
//Waveshaper array morphing
void wsMorph() {
	int i;
	for (i = 0; i < 65; i++) {
		newWaveform[i] = (waveformsTable[waveform_select1][i] * (ws_warp)) + (waveformsTable[waveform_select2][i] * (1 - ws_warp)) * (ws_drive);
	}
}

void LFOupdate(float LFOtop, float LFObottom) {
	static float LFO = 0;
	static unsigned long LFOtime = 0;
	static bool LFOdirection = false;
	unsigned long currentMicros = micros();
	static float LFOrange = 0;



	if (currentMicros - LFOtime >= LFOspeed) {
		LFOtime = currentMicros;
		LFOrange = LFOtop - LFObottom;
		if (LFOrange < 0) LFOrange = 0;


		L_glitch_mixer.gain(1, L_glitch_head2 * ((LFOrange * LFO) + LFOdepth));
		R_glitch_mixer.gain(1, R_glitch_head2 * (1 - ((LFOrange * LFO) + LFOdepth)));
		L_glitch_mixer.gain(2, L_glitch_head3 * (1 - ((LFOrange * LFO) + LFOdepth)));
		R_glitch_mixer.gain(2, R_glitch_head3 * ((LFOrange * LFO) + LFOdepth));


		// Update LFO
		if (LFOdirection == false) { //UP
			LFO = (LFO + 0.01);
			if (LFO >= 1) {
				LFOdirection = true;
			}
		}

		if (LFOdirection == true) { //Down
			LFO = (LFO - 0.01);
			if (LFO <= 0) {
				LFOdirection = false;
			}
		}
	}
}
