#include "Arduino.h"


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
