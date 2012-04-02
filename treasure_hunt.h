#include "sysio.h"
#include "tcvphys.h"
#include "phys_cc1100.h"
#include "plug_null.h"
#include "ser.h"
#include "serf.h"
#include "form.h"

int fd = -1;
int sid = 0;
int channel = 7;
int power = 10;
int currentSS = 0;
int blinkWait = 0;
int currentLight = -1;
char *outBuf, *inBuf;

#define MSG_SIZE    10
#define PAC_SIZE    20
#define HUNTER      "H"
#define TREASURE    "T"
#define RELAY       "R"
#define OFF         0
#define ON          1
#define BLINK       2
#define SEND_WAIT   512
#define NO_BSIG_CD  1536
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
