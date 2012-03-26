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

int fd = -1;
int sid = 0;
int channel = 0;
int power = 10;
int currentSS = 250;
int blinkWait = 0;
int currentLight = -1;
int triggerSend = 0;
char *outBuf, *inBuf;

#define SEND        (&triggerSend)
#define ROLE        "R"
#define MSG_SIZE    10
#define PAC_SIZE    20
#define TREASURE    "T"
#define NO_SIGNAL   2
#define READY       0
#define LIGHTS_OFF  do {			\
    leds (0, 0);				\
    leds (1, 0);				\
    leds (2, 0);				\
  } while (0)

void setBlinkRate (int);

/* use a function to calculate the flashing interval */
void setBlinkRate (int rssi) {
  /* set the current signal strength */
  currentSS = rssi;
  /* separate the strength into two to better indicate the distance */
  if (rssi >= 100)
    blinkWait = (3600 / rssi) * (3600 / rssi);
  else
    blinkWait = (4000 / rssi) * (4000 / (200 - rssi));
}

/* sender is a fsm that sends the role of the device every two seconds */
fsm sender {
  address packet;
  initial state SENDWAIT:
    triggerSend = 0;
    when (SEND, SENDSIGNAL);
    release;

  state SENDSIGNAL:
    packet = tcv_wnp (SENDSIGNAL, fd, PAC_SIZE);
    tcv_write (packet, outBuf, MSG_SIZE);
    tcv_endp (packet);
    proceed SENDWAIT;
}

/* receiver is a fsm that changes the status of the device accordingly */
fsm receiver {
  address packet;
  int n;
  initial state RECEIVING:
    delay (3072, NOSIGNAL);
    packet = tcv_rnp (RECEIVING, fd);
    n = tcv_left (packet);
    tcv_read (packet, inBuf, PAC_SIZE);
    tcv_endp (packet);
    proceed RECEIVED;
    release;

  state RECEIVED:
    if (strncmp(inBuf + 2, TREASURE, 1) == 0) {
      if (currentLight != READY) {
	LIGHTS_OFF;
	currentLight = READY;
	leds (currentLight, 2);
      }
      
      triggerSend = 1;
      trigger (SEND);
      /*
      // get the signal strength
      word rssi = (unsigned char) inBuf[n - 1];
      // change the stored signal strength if different
      if (rssi > 0 && currentSS != rssi)
	setBlinkRate (rssi);
      */
    }
 
    proceed RECEIVING;

  state NOSIGNAL:
    if (currentLight != NO_SIGNAL) {
      LIGHTS_OFF;
      currentLight = NO_SIGNAL;
      leds (currentLight, 1);
    }
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
    //    form (outBuf + 3, "%d", ss);

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
    runfsm sender;
    runfsm receiver;

    finish;
}


