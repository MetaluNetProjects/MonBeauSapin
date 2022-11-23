/*********************************************************************
 *               DMX master example for Versa1.0
 *	Output DMX frames to AUXSERIAL_TX  (K11 for Versa1.0)
 *********************************************************************/
#define BOARD Versa2
#include <fruit.h>
#include <dmx.h>
t_delay mainDelay;
int ledCount;

typedef struct _color_ {
	byte R;
	byte G;
	byte B;
} t_color;

//t_color[4] Palette;


//----------- Setup ----------------
void setup(void) {	
	fruitInit();
	pinModeDigitalOut(LED);
	DMXInit();        // init DMX master module
	delayStart(mainDelay, 10000); 	// init the mainDelay to 10 ms
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
	}

}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	int i;
	unsigned char chan, val, len;
	unsigned char c=fraiseGetChar(); // get first byte
	switch(c) {
		PARAM_INT(30,i); DMXSet(i, fraiseGetChar()); break; // if first byte is 30 then get DMX channel (int) and value (char).
		PARAM_CHAR(31, chan); len = fraiseGetLen() - 2;
			while(len--) {
				val = fraiseGetChar();
				DMXSet(chan, val);
				chan++;
			}
			break;
	}
}

