/* This is the treasure program that are supposed to be hidden
 * somewhere in one of the buildings and keeps sending out
 * signals indicating its location
 */

#include "sysio.h"
#include "tcvphys.h"
#include "phys_cc1100.h"
#include "plug_null.h"
#include "ser.h"
#include "serf.h"
#include "form.h"
#include "treasure_hunt.h"

int hunterInRange = 0;
int better = 0;

#define BETTER  (&better)
#define HUNTER_IN_RANGE  (&hunterInRange)
#define ROLE             TREASURE
#define NO_SIGNAL        1
#define GETTING_CLOSE    2
#define FAR_AWAY         0
#define WAIT_HUNTER      1536

/* timer fsm that keeps track of the wait time according
 * to the received packet 
 */
fsm waitTimer {
  // reset the timer when the desired packet is received
  initial state START:
    when (HUNTER_IN_RANGE, START);
    delay (WAIT_HUNTER, NOSIGNAL);
    release;    

  // when no proper packet is received
  state NOSIGNAL:
    if (blinkWait != 0)
      blinkWait = 0;

    when (HUNTER_IN_RANGE, START);
    release;
}

/* this fsm waits 2 seconds for a better signal to come
 * in, after that it gives up and starts to accept other
 * signal
 */
fsm waitBetterSig {
  initial state START:
    when (BETTER, START);
    delay (NO_BSIG_CD, DONEWAITING);
    release;

 state DONEWAITING:
    currentSS = 0;
    when (BETTER, START);
    release;
}

/* sender fsm that sends out the role of this node
 * every so often
 */
fsm sender {
  address packet;
  initial state SENDSIGNAL:
    packet = tcv_wnp (SENDSIGNAL, fd, PAC_SIZE);
    tcv_write (packet, outBuf, MSG_SIZE);
    tcv_endp (packet);

  state SENT:
    delay (SEND_WAIT, SENDSIGNAL);
    release;
}

/* receiver fsm decides if the received packet is a 
 * proper one and set the leds
 */
fsm receiver {
  address packet;
  int n;
  int wait;
  int rssi;

  state RECEIVING:
    packet = tcv_rnp (RECEIVING, fd);
    n = tcv_left (packet);
    tcv_read (packet, inBuf, PAC_SIZE);
    tcv_endp (packet);

  state RECEIVED:
    if (strncmp(inBuf + 2, HUNTER, 1) == 0) {
      // get the signal strength
      rssi = (unsigned char) inBuf[n - 1];
      if (rssi >= currentSS || currentSS - rssi <= 3) {
	trigger (BETTER);
	if (rssi != currentSS)
	  setBlinkRate (rssi);
	// set led colour accordingly
	if (rssi <= 150) {
	  if (currentLight != FAR_AWAY) {
	    LIGHTS_OFF;
	    currentLight = FAR_AWAY;
	  }
	}
	else {
	  if (currentLight != GETTING_CLOSE) {
	    LIGHTS_OFF;
	    currentLight = GETTING_CLOSE;
	  }
	}
      }
      // reset count down
      trigger (HUNTER_IN_RANGE);
    }

    proceed RECEIVING;
}

/* blinker fsm controls the led blinking rate according
 * to the precalculated rate by the setBlinkRate()
 * function 
 */
fsm blinker {
  initial state TURN_ON:
    if (blinkWait <= 0) {
      if (currentLight != NO_SIGNAL) {
	LIGHTS_OFF;
	currentLight = NO_SIGNAL;
	leds (currentLight, 1);
      }
      delay (512, TURN_ON);
      release;
    }
    else {
      leds (currentLight, 1);
      delay ((blinkWait / 3), TURN_OFF);
      release;
    }

  state TURN_OFF:
    LIGHTS_OFF;
    delay (blinkWait, TURN_ON);
    release;
}

/* initializing and start up */
fsm root {
  initial state INIT:
    // allocating memory for buffers
    outBuf = (char*) umalloc (MSG_SIZE);
    inBuf = (char*) umalloc (PAC_SIZE);
    // error checking
    if (outBuf == NULL || inBuf == NULL)
      diag ("fail to allocate memory for outBuf or inBuf");
    // initializing buffers
    bzero (outBuf, MSG_SIZE);
    bzero (inBuf, PAC_SIZE);
    form (outBuf + 2, ROLE, 1);
    
    phys_cc1100 (0, PAC_SIZE);
    tcv_plug (0, &plug_null);
    fd = tcv_open (NONE, 0, 0);
    // error checking
    if (fd < 0) {
      diag ("failed to obtain file descriptor");
      finish;
    }
    // initializing wireless parameter
    tcv_control (fd, PHYSOPT_SETSID, (address) &sid);
    tcv_control (fd, PHYSOPT_SETCHANNEL, (address) &channel);
    tcv_control (fd, PHYSOPT_SETPOWER, (address) &power);
    tcv_control (fd, PHYSOPT_TXON, NULL);
    tcv_control (fd, PHYSOPT_RXON, NULL);
    // initializing leds
    currentLight = NO_SIGNAL;
    leds (currentLight, 1);
    // delay up to 2 seconds and then start up
    delay (rnd() % 2048, STARTUP);
    release;

  state STARTUP:
    runfsm blinker;
    runfsm waitTimer;
    runfsm waitBetterSig;
    runfsm sender;
    runfsm receiver;
    
    finish;
}


