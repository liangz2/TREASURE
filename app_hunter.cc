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

int fd = -1;
int sid = 0;
int channel = 0;
int blinkWait = 0;
int power = 0;
int currentSS = 0;
int currentLight = -1;
char *outBuf, *inBuf;

#define ROLE        "H"
#define MSG_SIZE    10
#define PAC_SIZE    20
#define TREASURE    "T"
#define RELAY       "R"
#define T_SIGNAL    1
#define R_SIGNAL    2
#define NO_SIGNAL   0
#define LIGHTS_OFF  do {			\
    leds (0, 0);				\
    leds (1, 0);				\
    leds (2, 0);				\
  } while (0)

void setBlinkRate (int);

void setBlinkRate (int ss) {
  currentSS = ss;
  if (ss >= 100)
    blinkWait = (4000 / ss) * (4000 / ss);
  else
    blinkWait = (4000 / ss) * (4000 / (200 - ss));
}

fsm sender {
  address packet;

  initial state SENDSIGNAL:
    packet = tcv_wnp (SENDSIGNAL, fd, PAC_SIZE);
    tcv_write (packet, outBuf, MSG_SIZE);
    tcv_endp (packet);

  state SENT:
    //ser_outf (SENT, "sent: %s\n", outBuf + 2);
    delay (2048, SENDSIGNAL);
    release;
}

fsm receiver {
  address packet;
  int ss = 0;
  int i;
  int n;
  char c;
  initial state RECEIVING:
  //delay (1024, NOSIGNAL);
    packet = tcv_rnp (RECEIVING, fd);
    n = tcv_left (packet);
    tcv_read (packet, inBuf, n);
    tcv_endp (packet);
    proceed RECEIVED;
    //release;

  state RECEIVED:
    if (strncmp(inBuf + 2, TREASURE, 1) == 0) {
      if (currentLight != T_SIGNAL) {
	currentLight = T_SIGNAL;
	leds (currentLight, 1);
      }
      /*
      // get the signal strength
      ss = atoi (inBuf[31]);
      // change the stored signal strength if different
      if (ss > 0 && currentSS != ss)
	setBlinkRate (ss);
      */
      for (i = 0; i < n; i++) {
	diag ("%d", inBuf[i]);
      }
      
      proceed RECEIVING;
    }

  state OUT_PUT:
    ser_outf (OUT_PUT, "%d\n\r", n);
    proceed RECEIVING;

  state NOSIGNAL:
    blinkWait = 0;
    proceed RECEIVING;
}

fsm blinker {
  initial state TURN_ON:
  //   diag ("%d", blinkWait);
    if (blinkWait <= 0) {
      LIGHTS_OFF;
      delay (512, TURN_ON);
      release;
    }
    else {
      leds (currentLight, 1);
      delay (blinkWait, TURN_OFF);
      release;
    }

  state TURN_OFF:
    leds (currentLight, 0);
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
    diag ("on");
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
    runfsm blinker;

    finish;
}


