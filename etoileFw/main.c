/*********************************************************************
 *               DMX master example for Versa
 *	Output DMX frames to AUXSERIAL_TX  (K11 for Versa)
 *********************************************************************/
#define BOARD Versa2
#include <fruit.h>
#include <dmx.h>
t_delay mainDelay;
int ledCount;

byte etoileR[150];
byte etoileG[150];
byte etoileB[150];

int etoileRout1[75];
int etoileGout1[75];
int etoileBout1[75];
int etoileRout2[75];
int etoileGout2[75];
int etoileBout2[75];

byte ledMove;
byte ledMoveStart = 9;
int flashProba;

typedef struct _color_ {
	byte R;
	byte G;
	byte B;
} t_color;

int gains[6] = {256, 256, 256, 256, 256, 256};


//----------- Setup ----------------
void setup(void) {
	fruitInit();
	pinModeDigitalOut(LED);
	DMXInit();        // init DMX master module
	delayStart(mainDelay, 10000); 	// init the mainDelay to 10 ms
}

int computeGain(int led) {
	byte index;

	//led = (led - ledMoveStart - 12 + 141) % 141;
	led = (led - ledMoveStart - 12 + 141);
	while(led >= 141) led -= 141;
	index = (11 * led) >> 8;
	if(index == 6) index = 5;
	//led = (2 * (led - ledMoveStart + 12 + 141)) % 47;
	//led = (2 * led) % 47;
	led = (led * 9) % 256;
	//index 
	//return (11UL * gains[index] * (23 - abs(23 - led))) >> 8;
	return (/*11UL **/ gains[index] * (128 - abs(128 - led))) >> 8;
}

#define FADE_FILTER 2

#if 1
#define FADELED(outchan, outtab, inchan, intab) do { \
		outtab[outchan] = outtab[outchan] - (outtab[outchan] >> FADE_FILTER) + intab[inchan]; \
		/*if(outtab[outchan] < intab[inchan] ) outtab[outchan] = outtab[outchan] + 1; \
		else if(outtab[outchan] > intab[inchan]) outtab[outchan] = outtab[outchan] - 1;*/\
	} while(0)
#else
#define FADELED(outchan, outtab, inchan, intab) do { \
		outtab[outchan] = intab[inchan] << FADE_FILTER; \
	} while(0)
#endif

void fadeLeds() {
	static byte chan;
	byte loops = 4, chan2;
	while(loops--) {
		if(chan < 75) {
			FADELED(chan, etoileRout1, chan, etoileR);
			FADELED(chan, etoileGout1, chan, etoileG);
			FADELED(chan, etoileBout1, chan, etoileB);
		} else {
			chan2 = chan - 75;
			FADELED(chan2, etoileRout2, chan, etoileR);
			FADELED(chan2, etoileGout2, chan, etoileG);
			FADELED(chan2, etoileBout2, chan, etoileB);
		}
		chan++;
		if(chan == 150) chan = 0; 
	}
}

#define COMPUTE_GAIN 0
void copyColor() {
	unsigned int c = 1;
	unsigned char led = 0;
	int ledMoved;
	int gain = gains[0];
	t_time start = time();
	while (led < 150) {
		if(led >= ledMoveStart) {
			ledMoved = ((led - 9) + ledMove);// % 141;
			while(ledMoved >= 141) ledMoved -= 141;
			ledMoved += 9;
			#if COMPUTE_GAIN
			gain = computeGain(ledMoved);
			#else
			gain = gains[0];
			#endif
		} else {
			ledMoved = led;
			gain = 256;
		}

		if((rand() >> 14) < flashProba) {
			DMXSet(c++, 255);
			DMXSet(c++, 255);
			DMXSet(c++, 255);
		} else {
//#define setLed(col, num) DMXSet(c++, ((unsigned int)gain * (etoile##col##out##num[ledMoved] >> FADE_FILTER) >> 8));
#define setLed(col, num) DMXSet(c++, etoile##col##out##num[ledMoved] >> FADE_FILTER);
			if(ledMoved < 75) {
				setLed(R, 1);
				setLed(G, 1);
				setLed(B, 1);
			} else {
				ledMoved -= 75;
				setLed(R, 2);
				setLed(G, 2);
				setLed(B, 2);
			}
		}
		led++;
		fraiseService();// listen to Fraise events
		DMXService();	// DXM management routine
	}
	
	//printf("Ct %d\n", elapsed(start));
}

// ---------- Main loop ------------
void loop() {
	fraiseService();// listen to Fraise events
	DMXService();	// DXM management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 10000); 	// re-init mainDelay
		if(ledCount++ == 25) {
			ledCount = 0;
			if(digitalRead(LED)) digitalClear(LED); else digitalSet(LED);
		}
		copyColor();
		fadeLeds();
	}

}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	int i, chan;
	unsigned char val;
	char len;
	unsigned char c=fraiseGetChar(); // get first byte
	switch(c) {
		PARAM_INT(30,i); DMXSet(i, fraiseGetChar()); break; // if first byte is 30 then get DMX channel (int) and value (char).
		PARAM_INT(31, chan); len = fraiseGetLen() - 3;
			while(len >= 3) {
				etoileR[chan] = fraiseGetChar();
				etoileG[chan] = fraiseGetChar();
				etoileB[chan] = fraiseGetChar();
				chan++;
				len -= 3;
			}
			break;
		PARAM_INT(32, ledMove); break;
		PARAM_INT(33, flashProba); break;
		case 34: 
			gains[0] = fraiseGetInt();
			gains[1] = fraiseGetInt();
			gains[2] = fraiseGetInt();
			gains[3] = fraiseGetInt();
			gains[4] = fraiseGetInt();
			gains[5] = fraiseGetInt();
			break;
	}
}

