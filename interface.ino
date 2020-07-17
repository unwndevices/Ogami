#include <Audio.h>
#include <ResponsiveAnalogRead.h>
#include <Bounce.h>
#include "waveforms.h"
#include "effect_glitchdelay.h"
#include "src/effect_dynamics/effect_dynamics.h"

void setupPins() {
	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(MZ, INPUT);
	pinMode(DZ, INPUT);
	pinMode(PINarcade, INPUT);
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
}

void midiCCread(byte channel, byte control, byte value) {
	switch (control) {
	case CCinGain:
		inGain();
		break;

	case CCfeedbackGain:
		feedbackGain();
		break;

	case CClowpassFreq:
		lowpassFreq();
		break;

	case CChighpassFreq:
		highpassFreq();
		break;


	case CCoutVol:
		outVol();
		break;

	case CCwsWarp:
		wsWarp();
		break;

	case CCwsDrive:
		wsDrive();
		break;

	case CCbcRate:
		bcRate();
		break;

	case CCbcBits:
		bcBits();
		break;

	case CCdriveWet:
		driveWet();
		break;

	case CCglitchSize:
		glitchSize();
		break;

	case CCglitchJitter:
		glitchJitter();
		break;

	case CCglitchHead1:
		glitchHead1();
		break;

	case CCglitchHead2:
		glitchHead2();
		break;

	case CCglitchHead3:
		glitchHead3();
		break;

	case CCglitchHead4:
		glitchHead4();
		break;

	case CCglitchWet:
		glitchWet();
		break;

	case CCwaveformSelect1:
		waveform_select1 = value;
		break;

	case CCwaveformSelect2: 
		waveform_select2 = value;
		break;



	}
}

void interfaceRead(){
	if (mux1Updated>muxTimeMin) {
		muxAnalogRead();
		//muxDigitalRead(); //fix hardware first
		arcadeRead();
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

//******ANALOG MULTIPLEXING CODE******//
void setupAnalog() {
	for (int i = 0; i < MUX_PINS; i++) {
		analog[i] = new ResponsiveAnalogRead(MZ, true); // initialize
	}
}


void muxAnalogRead(){
	analog[i]->update();
	// if the repsonsive value has change, print out 'changed'
	if(analog[i]->hasChanged()) {
		data[i] = analog[i]->getValue()>>3;
		if (data[i] != dataLag[i]) {
			dataLag[i] = data[i];
			switch (i)
			{
			case MUX0:
				value = data[i];
				outVol();
				break;

			case MUX1:
				value = data[i];
				feedbackGain();
				break;

			case MUX2:
				value = data[i];
				glitchHead2();
				glitchHead3();
				break;

			case MUX3:
				value = data[i];
				glitchSize();
				break;

			case MUX4:
				value = data[i];
				glitchHead1();
				break;

			case MUX5:
				value = data[i];
				glitchJitter();
				break;

			case MUX6:
				value = data[i];
				glitchWet();
				break;
			}
		}
	}
}

//******DIGITAL MULTIPLEXING CODE******//
void setupDigital() {
	for (int i = 0; i < MUX_PINS; i++) {
		digital[i] = new Bounce(DZ, bounceTime); // initialize
	}
}

void muxDigitalRead(){
	for (int i=0; i<MUX_PINS; i++) {
		digital[i]->update();
		if (digital[MUXDswitch1_A]->risingEdge()) {
		}
		else if (digital[MUXDswitch1_B]->risingEdge()) {
		}
		else {
		}

		if (digital[MUXDswitch2_A]->risingEdge()) {
		}
		else if (digital[MUXDswitch2_B]->risingEdge()) {
		}
		else {}

		if (digital[MUXDswitch3_A]->risingEdge()) {
		}
		else if (digital[MUXDswitch3_B]->risingEdge()) {
		}
		else {
		}

		if (digital[MUXDswitch4_A]->risingEdge()) {
		}
		else if (digital[MUXDswitch4_B]->risingEdge()) {
		}
		else {}
	}
}

void arcadeRead(){
	arcade.update();
	if (arcade.risingEdge()) {
		glitch.set_freeze_active(true);
		digitalWrite(led1, HIGH);
	}
	if (arcade.fallingEdge()) {
		glitch.set_freeze_active(false);
		digitalWrite(led1, LOW);
	}
}
