#include <Audio.h>
#include <ResponsiveAnalogRead.h>
#include <Bounce.h>
#include "config.h"
#include "waveforms.h"
#include "src/effect_glitchdelay/effect_glitchdelay.h"
#include "src/effect_dynamics/effect_dynamics.h"

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
AudioConnection patchCord6(waveshaper, 0, bitcrusher, 0);
AudioConnection patchCord7(lowpassF, 0, highpassF, 0);
AudioConnection patchCord8(bitcrusher, 0, drive_mixer, 1);
AudioConnection patchCord8a(highpass, 2, drive_mixer, 0);
AudioConnection patchCord9(drive_mixer, 0, compressor, 0);
AudioConnection patchCord10(compressor, 0, glitch, 0);
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
AudioConnection patchCord24(R_dry_wet_mixer, 0, R_gain, 0);
AudioConnection patchCord25(L_dry_wet_mixer, 0, L_gain, 0);
AudioConnection patchCord26(L_gain, 0, out, 0);
AudioConnection patchCord27(R_gain, 0, out, 1);
AudioControlSGTL5000 sgtl5000;           //xy=672,1106




void setup() {
	// patchA1.disconnect(); //disconnect patch cables
	AudioMemory(22);
	setupAudio();
	setupUnwn();
	//MIDI SETUP
	usbMIDI.setHandleControlChange(midiCCread);
	// usbMIDI.setHandleClock(myClock); //MIDI SYNC
	// usbMIDI.setHandleStart(myStart);
	// usbMIDI.setHandleContinue(myContinue);
	// usbMIDI.setHandleStop(myStop);

}

void loop() {
	interfaceRead();
	LFOupdate(LFOrangeTop, LFOdepth);
	while (usbMIDI.read()) {
		// controllers must call .read() to keep the queue clear even if they are not responding to MIDI
	}
}

void setupAudio () {
	sgtl5000.enable();
	sgtl5000.volume(0.8);

	bitcrusher.sampleRate(44100);
	bitcrusher.bits(16);

	waveshaper.shape(waveformsTable[0], 65);

	drive_mixer.gain(0, 0);
	drive_mixer.gain(1, 1);

	compressor.limit(-2.0f, 0.005f, 0.01f); //limit(threshold = -6.0f, attack = 0.005f, release = 0.01f)
	compressor.compression(-12.0f, 0.05f, 0.2f, 5.0f, 7.0f); //compression(threshold = -35.0f, attack = 0.005f, release = 0.2f, ratio = 45.0f, kneeWidth = 6.0f
	compressor.autoMakeupGain(6.0f);

	feedback_mixer.gain(0, 1);
	feedback_mixer.gain(1, 0);

	lowpass.frequency(15000);
	lowpass.resonance(0.7);
	lowpass.octaveControl(0);

	lowpassF.frequency(9000);
	lowpassF.resonance(0.7);
	lowpassF.octaveControl(0);

	highpass.frequency(30);
	highpass.resonance(0.7);
	highpass.octaveControl(0);

	highpassF.frequency(300);
	highpassF.resonance(0.7);
	highpassF.octaveControl(0);

	L_glitch_mixer.gain(0, 0.7);
	L_glitch_mixer.gain(1, 0.6);
	L_glitch_mixer.gain(2, 0.5);
	L_glitch_mixer.gain(3, 0);
	R_glitch_mixer.gain(0, 0.7);
	R_glitch_mixer.gain(1, 0.55);
	R_glitch_mixer.gain(2, 0.55);
	R_glitch_mixer.gain(3, 0);

	L_dry_wet_mixer.gain(0, 0);
	L_dry_wet_mixer.gain(1, 1);
	R_dry_wet_mixer.gain(0, 0);
	R_dry_wet_mixer.gain(1, 1);

	L_gain.gain(1.2f);
	R_gain.gain(1.2f);

	glitch.set_loop_size( 0, delay_time );
	glitch.set_loop_size( 1, delay_time );
	glitch.set_loop_size( 2, delay_time );
	glitch.set_loop_size( 3, delay_time );

	glitch.set_jitter( 0, 0.2 );
	glitch.set_jitter( 1, 0.2 );
	glitch.set_jitter( 2, 0.2 );
	glitch.set_jitter( 3, 0.2 );

	glitch.set_loop_moving(false);
}
