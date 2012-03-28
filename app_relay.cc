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

int ptSignal = 0;

#define P_T_SIGNAL  (&ptSignal)
#define ROLE        RELAY
#define READY       0
#define NO_SIGNAL   2
#define WAIT_SIGNAL 1536

/* timer fsm that keeps track of the wait time according
 * to the received packet 
 */
fsm waitTimer {
  // reset the timer when the desired packet is received
  initial state START:
    when (P_T_SIGNAL, START);
    delay (WAIT_SIGNAL, NOSIGNAL);
    release;    

  // when no proper packet is received
  state NOSIGNAL:
    if (currentLight != NO_SIGNAL) {
      LIGHTS_OFF;
      currentLight = NO_SIGNAL;
      leds (currentLight, ON);
    }

    when (P_T_SIGNAL, START);
    release;
}

/* sender fsm that sends out the role of this node
 * every so often
 */
fsm sender {
  address packet;
  initial state SENDWAIT:
    when (P_T_SIGNAL, SENDSIGNAL);
    release;

  state SENDSIGNAL:
    packet = tcv_wnp (SENDSIGNAL, fd, PAC_SIZE);
    tcv_write (packet, outBuf, MSG_SIZE);
    tcv_endp (packet);
    proceed SENDWAIT;
}

/* receiver is a fsm that changes the status of the 
 * device accordingly
 */
fsm receiver {
  address packet;
  initial state RECEIVING:
    packet = tcv_rnp (RECEIVING, fd);
    tcv_read (packet, inBuf, PAC_SIZE);
    tcv_endp (packet);
    
  state RECEIVED:
    if (strncmp(inBuf + 2, TREASURE, 1) == 0) {
      if (currentLight != READY) {
	LIGHTS_OFF;
	currentLight = READY;
	leds (currentLight, BLINK);
      }
      
      // trigger the picked up signal flag
      trigger (P_T_SIGNAL);
      /*
      // get the signal strength
      word rssi = (unsigned char) inBuf[n - 1];
      // change the stored signal strength if different
      if (rssi > 0 && currentSS != rssi)
	setBlinkRate (rssi);
      */
    }
 
    proceed RECEIVING;
}
/*
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
*/
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
    leds (currentLight, ON);

    delay (rnd() % 2048, STARTUP);
    release;

  state STARTUP:
    runfsm sender;
    runfsm receiver;
    runfsm waitTimer;

    finish;
}


