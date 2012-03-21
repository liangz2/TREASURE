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
int power = 0;
int ss = 250;
char *outBuf, *inBuf;

#define ROLE        "T"
#define MSG_SIZE    26
#define PAC_SIZE    30
#define HUNTER      "H"

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
    if (strncmp(inBuf + 2, HUNTER, 1) == 0) {
      ser_out (RECEIVED, "received signal from hunter\n");
      leds (0, 2);
    }
    proceed RECEIVING;
}

fsm root {
  initial state INIT:
    outBuf = (char*) umalloc (MSG_SIZE);
    inBuf = (char*) umalloc (MSG_SIZE);

    if (outBuf == NULL || inBuf == NULL)
      diag ("fail to allocate memory for outBuf or inBuf");

    bzero (outBuf, MSG_SIZE);
    form (outBuf + 2, ROLE, 1);
    form (outBuf + 3, "%d", ss);

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


