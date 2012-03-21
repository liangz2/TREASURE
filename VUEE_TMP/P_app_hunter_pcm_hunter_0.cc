#include "__vueehdr_hunter.h"
# 13 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

#define fd (((__NT_hunter*)TheStation)->__vattr_fd)
#define sid (((__NT_hunter*)TheStation)->__vattr_sid)
#define channel (((__NT_hunter*)TheStation)->__vattr_channel)
#define blinkWait (((__NT_hunter*)TheStation)->__vattr_blinkWait)
#define power (((__NT_hunter*)TheStation)->__vattr_power)
#define currentSS (((__NT_hunter*)TheStation)->__vattr_currentSS)
#define currentLight (((__NT_hunter*)TheStation)->__vattr_currentLight)
#define outBuf (((__NT_hunter*)TheStation)->__vattr_outBuf)
#define inBuf (((__NT_hunter*)TheStation)->__vattr_inBuf)
# 21 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"












void setBlinkRate (int);

void setBlinkRate (int ss) {
  currentSS = ss;
  if (ss >= 100)
    blinkWait = (4000 / ss) * (4000 / ss);
  else
    blinkWait = (4000 / ss) * (4000 / (200 - ss));
}


#define packet (((__NT_hunter*)TheStation)->__vattr_sender_packet)
# 43 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
sender_hunter::perform { _pp_enter_ (); 

# 43 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  

  transient SENDSIGNAL: {
    packet = tcv_wnp (SENDSIGNAL, fd, 30);
    tcv_write (packet, outBuf, 30);
    tcv_endp (packet);

  } transient SENT: {

    delay (2048, SENDSIGNAL);
    release;
}}
#undef packet

#define packet (((__NT_hunter*)TheStation)->__vattr_receiver_packet)
#define ss (((__NT_hunter*)TheStation)->__vattr_receiver_ss)
# 57 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
receiver_hunter::perform { _pp_enter_ (); 

# 57 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  
  
  transient RECEIVING: {
    packet = tcv_rnp (RECEIVING, fd);
    tcv_read (packet, inBuf, 30);
    tcv_endp (packet);

  } transient RECEIVED: {
    if (strncmp(inBuf + 2, "T", 1) == 0) {
      if (currentLight != 1) {
 currentLight = 1;
 do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (currentLight, 1); } while (0);
      }

      ss = atoi (inBuf + 3);

      if (ss > 0 && currentSS != ss)
 setBlinkRate (ss);

      proceed RECEIVING;
    }

  } transient OUT_PUT: {
    ser_outf (OUT_PUT, "%d\n", blinkWait);
    proceed RECEIVING;
}}
#undef packet
#undef ss
# 83 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"



# 85 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
blinker_hunter::perform { _pp_enter_ (); 

# 85 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  transient TURN_ON: {
  if (blinkWait <= 0) {
      do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (0, 0); } while (0); do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (1, 0); } while (0); do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (2, 0); } while (0);
      delay (512, TURN_ON);
      release;
    }
    else {
      do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (currentLight, 1); } while (0);
      delay (blinkWait, TURN_OFF);
      release;
    }

  } transient TURN_OFF: {
    do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (currentLight, 0); } while (0);
    delay (blinkWait, TURN_ON);
    release;
}}


# 104 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
root_hunter::perform { _pp_enter_ (); 

# 104 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  transient INIT: {
    outBuf = (char*) (((PicOSNode*)TheStation)->memAlloc (26, (word)(26)));
    inBuf = (char*) (((PicOSNode*)TheStation)->memAlloc (26, (word)(26)));

    if (outBuf == 0 || inBuf == 0)
      diag ("fail to allocate memory for outBuf or inBuf");

    bzero (outBuf, 26);
    form (outBuf + 2, "H", 1);
    form (outBuf + 3, "%s", "hunter is here");

    phys_cc1100 (0, 30);
    tcv_plug (0, &plug_null);
    fd = tcv_open ((-1), 0, 0);

    if (fd < 0) {
      diag ("failed to obtain file descriptor");
      finish;
    }

    tcv_control (fd, 10, (address) &sid);
    tcv_control (fd, 18, (address) &channel);
    tcv_control (fd, 7, (address) &power);
    tcv_control (fd, 1, 0);
    tcv_control (fd, 4, 0);

    delay (rnd() % 2048, STARTUP);
    release;

  } transient STARTUP: {
    (TheNode->tally_in_pcs()?(create sender_hunter )->_pp_apid_():0);
    (TheNode->tally_in_pcs()?(create receiver_hunter )->_pp_apid_():0);
    (TheNode->tally_in_pcs()?(create blinker_hunter )->_pp_apid_():0);

    finish;
}}

void __NT_hunter::__praxis_starter () { (TheNode->tally_in_pcs()?(create root_hunter )->_pp_apid_():0); }
