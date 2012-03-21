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
# 47 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
sender_hunter::perform { _pp_enter_ (); 

# 47 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  

  transient SENDSIGNAL: {
    packet = tcv_wnp (SENDSIGNAL, fd, 20);
    tcv_write (packet, outBuf, 10);
    tcv_endp (packet);

  } transient SENT: {

    delay (2048, SENDSIGNAL);
    release;
}}
#undef packet

#define packet (((__NT_hunter*)TheStation)->__vattr_receiver_packet)
#define ss (((__NT_hunter*)TheStation)->__vattr_receiver_ss)
#define i (((__NT_hunter*)TheStation)->__vattr_receiver_i)
#define n (((__NT_hunter*)TheStation)->__vattr_receiver_n)
#define c (((__NT_hunter*)TheStation)->__vattr_receiver_c)
# 61 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
receiver_hunter::perform { _pp_enter_ (); 

# 61 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  
  
  
  
  
  transient RECEIVING: {

    packet = tcv_rnp (RECEIVING, fd);
    n = tcv_left (packet);
    tcv_read (packet, inBuf, n);
    tcv_endp (packet);
    proceed RECEIVED;


  } transient RECEIVED: {
    if (strncmp(inBuf + 2, "T", 1) == 0) {
      if (currentLight != 1) {
 currentLight = 1;
 do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (currentLight, 1); } while (0);
      }







      for (i = 0; i < n; i++) {
 diag ("%d", inBuf[i]);
      }

      proceed RECEIVING;
    }

  } transient OUT_PUT: {
    ser_outf (OUT_PUT, "%d\n\r", n);
    proceed RECEIVING;

  } transient NOSIGNAL: {
    blinkWait = 0;
    proceed RECEIVING;
}}
#undef packet
#undef ss
#undef i
#undef n
#undef c
# 103 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"



# 105 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
blinker_hunter::perform { _pp_enter_ (); 

# 105 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  transient TURN_ON: {

    if (blinkWait <= 0) {
      do { do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (0, 0); } while (0); do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (1, 0); } while (0); do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (2, 0); } while (0); } while (0);
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


# 125 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
root_hunter::perform { _pp_enter_ (); 

# 125 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"

  transient INIT: {
    outBuf = (char*) (((PicOSNode*)TheStation)->memAlloc (10, (word)(10)));
    inBuf = (char*) (((PicOSNode*)TheStation)->memAlloc (20, (word)(20)));

    if (outBuf == 0 || inBuf == 0)
      diag ("fail to allocate memory for outBuf or inBuf");

    bzero (outBuf, 10);
    bzero (inBuf, 20);
    form (outBuf + 2, "H", 1);

    phys_cc1100 (0, 20);
    tcv_plug (0, &plug_null);
    fd = tcv_open ((-1), 0, 0);

    if (fd < 0) {
      diag ("failed to obtain file descriptor");
      finish;
    }
    diag ("on");
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
