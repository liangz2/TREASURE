/* This is the hunter program that are used to find the  hidden
 * treasure in one of the buildings and keeps sending out
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

int relayFound = 0;
int inRange = 0;
int better = 0;

#define BETTER      (&better)
#define IN_RANGE    (&inRange)
#define ROLE        HUNTER
#define T_SIGNAL    1
#define R_SIGNAL    0
#define NO_SIGNAL   2
#define WAIT_SIGNAL 1536
#define NO_BSIG_CD  2048

/* timer fsm that keeps track of the wait time according
 * to the received packet 
 */
fsm waitTimer {
  // reset the timer when the desired packet is received
  initial state START:
    when (IN_RANGE, START);
    delay (WAIT_SIGNAL, NOSIGNAL);
    release;    

  // when no proper packet is received
  state NOSIGNAL:
    if (blinkWait != 0)
      blinkWait = 0;

    when (IN_RANGE, START);
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

    delay (SEND_WAIT, SENDSIGNAL);
    release;
}

/* receiver fsm decides if the received packet is a 
 * proper one and set the leds
 */
fsm receiver {
  address packet;
  int n;
  word rssi;
  
  initial state RECEIVING:
    packet = tcv_rnp (RECEIVING, fd);
    n = tcv_left (packet);
    tcv_read (packet, inBuf, n);
    tcv_endp (packet);

  state RECEIVED:
    if (strncmp(inBuf + 2, TREASURE, 1) == 0 &&
	relayFound == 1) {
      if (currentLight != T_SIGNAL) {
	LIGHTS_OFF;
	currentLight = T_SIGNAL;
      }
      proceed SETBLINK;
    }
    else if (strncmp(inBuf + 2, RELAY, 1) == 0 &&
	     relayFound == 0) {
      if (currentLight != R_SIGNAL) {
	LIGHTS_OFF;
	currentLight = R_SIGNAL;
      }
      proceed SETBLINK;
    }
    
    proceed RECEIVING;
    

  state SETBLINK:
    // get the signal strength
    rssi = (unsigned char) inBuf[n - 1];
    // set the relayFound flag to true if the hunter gets close enough
    if (rssi >= 230 && relayFound == 0) {
      relayFound = 1;
      runfsm sender;
    }
    // change the stored signal strength if different
    if (rssi >= currentSS || currentSS - rssi <= 3) {
      trigger (BETTER);
      if (rssi != currentSS)
	setBlinkRate (rssi);
    }

    trigger (IN_RANGE);
    
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
	leds (currentLight, ON);
      }
      delay (512, TURN_ON);
      release;
    }
    else {
      leds (currentLight, ON);
      delay ((blinkWait / 3), TURN_OFF);
      release;
    }

  state TURN_OFF:
    LIGHTS_OFF;
    delay (blinkWait, TURN_ON);
    release;
}

fsm root {
  initial state INIT:
    outBuf = (char*) umalloc (MSG_SIZE);
    inBuf = (char*) umalloc (PAC_SIZE);

    if (outBuf == NULL || inBuf == NULL)
      diag ("fail to allocate memory for outBuf or inBuf");

    bzero (outBuf, MSG_SIZE);
    bzero (inBuf, PAC_SIZE);
    form (outBuf + 2, ROLE, 1);

    phys_cc1100 (0, PAC_SIZE);
    tcv_plug (0, &plug_null);
    fd = tcv_open (NONE, 0, 0);

    if (fd < 0) {
      diag ("failed to obtain file descriptor");
      finish;
    }
    
    tcv_control (fd, PHYSOPT_SETSID, (address) &sid);
    tcv_control (fd, PHYSOPT_SETCHANNEL, (address) &channel);
    tcv_control (fd, PHYSOPT_SETPOWER, (address) &power);
    tcv_control (fd, PHYSOPT_TXON, NULL);
    tcv_control (fd, PHYSOPT_RXON, NULL);
    
    // initialize current light to red and stay on
    currentLight = NO_SIGNAL;
    leds (currentLight, ON);

    // delay random time within 2 seconds to start
    delay (rnd() % 2048, STARTUP);
    release;

  state STARTUP:
    runfsm receiver;
    runfsm blinker;
    runfsm waitTimer;
    runfsm waitBetterSig;

    finish;
}


