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
int blinkWait = -1;
int power = 0;
int currentLight = -1;
char *outBuf, *inBuf;

#define ROLE        "H"
#define MSG_SIZE    26
#define PAC_SIZE    30
#define TREASURE    "T"
#define RELAY       "R"
#define T_SIGNAL    1
#define R_SIGNAL    2
#define NO_SIGNAL   0

fsm sender {
  address packet;

  initial state SENDSIGNAL:
    packet = tcv_wnp (SENDSIGNAL, fd, PAC_SIZE);
    tcv_write (packet, outBuf, PAC_SIZE - 2);
    tcv_endp (packet);

  state SENT:
    ser_outf (SENT, "sent: %s\n", outBuf + 2);
    delay (2048, SENDSIGNAL);
    release;
}

fsm receiver {
  address packet;
  initial state RECEIVING:
    packet = tcv_rnp (RECEIVING, fd);
    tcv_read (packet, inBuf, PAC_SIZE);
    tcv_endp (packet);

  state RECEIVED:
    if (strncmp(inBuf + 2, TREASURE, 1) == 0) {
      if (currentLight != T_SIGNAL) {
	currentLight = T_SIGNAL;
	leds (currentLight, 1);
      }
    }

    proceed RECEIVING;
}

fsm blinker {
  state ON:
    if (blinkWait < 0)
      leds (NO_SIGNAL, 0);
    else
      leds (currentLight, 1);
    
    delay (blinkWait, off)
      
}

void setBlinkRate (int ss) {
  if (ss >= 100)
    blinkWait = (4000 / ss) * (4000 / ss);
  else
    blinkWait = (4000 / ss) * (4000 / (200 - ss));
}

fsm root {
  initial state INIT:
    outBuf = (char*) umalloc (MSG_SIZE);
    inBuf = (char*) umalloc (MSG_SIZE);

    if (outBuf == NULL || inBuf == NULL)
      diag ("fail to allocate memory for outBuf or inBuf");

    bzero (outBuf, MSG_SIZE);
    form (outBuf + 2, ROLE, 1);
    form (outBuf + 3, "%s", "hunter is here");

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
    
    delay (rnd() % 2048, STARTUP);
    release;

  state STARTUP:
    runfsm sender;
    runfsm receiver;

    finish;
}


