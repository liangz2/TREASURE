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
char *outBuf, *inBuf;

#define ROLE        "T"
#define MSG_SIZE    10
#define PAC_SIZE    20
#define HUNTER      "H"
#define NO_SIGNAL    1
#define EXPOSED     2
#define YELLOW      0
#define LIGHTS_OFF  do {			\
    leds (0, 0);				\
    leds (1, 0);				\
    leds (2, 0);				\
  } while (0)

void setBlinkRate (int);

void setBlinkRate (int rssi) {
  currentSS = rssi;
  if (rssi >= 100)
    blinkWait = (3600 / rssi) * (3600 / rssi);
  else
    blinkWait = (4000 / rssi) * (4000 / (200 - rssi));
}

fsm sender {
  address packet;
  initial state SENDSIGNAL:
    packet = tcv_wnp (SENDSIGNAL, fd, PAC_SIZE);
    tcv_write (packet, outBuf, MSG_SIZE);
    tcv_endp (packet);

  state SENT:
    delay (2048, SENDSIGNAL);
    release;
}

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
    if (strncmp(inBuf + 2, HUNTER, 1) == 0) {
      if (currentLight != EXPOSED) {
	leds (currentLight, 0);
	currentLight = EXPOSED;
	leds (currentLight, 2);
      }
      
      // get the signal strength
      word rssi = (unsigned char) inBuf[n - 1];
      // change the stored signal strength if different
      if (rssi > 0 && currentSS != rssi)
	setBlinkRate (rssi);
    }
 
    proceed RECEIVING;

  state NOSIGNAL:
    blinkWait = 0;
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
    runfsm blinker;
    runfsm sender;
    runfsm receiver;

    finish;
}


