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
int waitBetterSig = 0;

#define WAIT_BETTER_SIG  (&waitBetterSig)
#define HUNTER_IN_RANGE  (&hunterInRange)
#define ROLE             TREASURE
#define NO_SIGNAL        1
#define GETTING_CLOSE    2
#define FAR_AWAY         0
#define WAIT_HUNTER      1536
#define WAIT_BETTER_SIG  2048

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
    delay (WAIT_BETTER_SIG, DONEWAITING);
    release;

 state DONEWAITING:
    waitBetterSig = 0;
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

  state RECEIVING:
    packet = tcv_rnp (RECEIVING, fd);
    n = tcv_left (packet);
    tcv_read (packet, inBuf, PAC_SIZE);
    tcv_endp (packet);

  state RECEIVED:
    if (strncmp(inBuf + 2, HUNTER, 1) == 0) {
      if (currentLight != GETTING_CLOSE) {
	LIGHTS_OFF;
	currentLight = GETTING_CLOSE;
	leds (currentLight, BLINK);
      }
      proceed SETBLINK;
    }

  state SETBLINK:
    // get the signal strength
    word rssi = (unsigned char) inBuf[n - 1];
    // change the stored signal strength if different
    if (waitBetterSig == 0) {
      setBlinkRate (rssi);
    }

    if (rssi > currentSS) {
      waitBetterSig = 1;
      trigger (WAIT_BETTER_SIG);
      setBlinkRate (rssi);
    }
    else {
      if (waitBetterSig == 0)
	setBlinkRate (rssi);
    }
      
    trigger (HUNTER_IN_RANGE);

    proceed RECEIVING;
}

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
    
    currentLight = NO_SIGNAL;
    leds (currentLight, 1);

    delay (rnd() % 2048, STARTUP);
    release;

  state STARTUP:
    runfsm blinker;
    runfsm waitTimer;
    runfsm sender;
    runfsm receiver;
    
    finish;
}


