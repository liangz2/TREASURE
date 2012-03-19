#include "__vueehdr_hunter.h"
# 13 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"

#define fd (((__NT_hunter*)TheStation)->__vattr_fd)
#define sid (((__NT_hunter*)TheStation)->__vattr_sid)
#define channel (((__NT_hunter*)TheStation)->__vattr_channel)
#define power (((__NT_hunter*)TheStation)->__vattr_power)
#define outBuf (((__NT_hunter*)TheStation)->__vattr_outBuf)
#define inBuf (((__NT_hunter*)TheStation)->__vattr_inBuf)






#define packet (((__NT_hunter*)TheStation)->__vattr_sender_packet)
# 25 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"
sender_hunter::perform { _pp_enter_ (); 

# 25 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"

  

  transient SENDSIGNAL: {
    packet = tcv_wnp (SENDSIGNAL, fd, 30);
    tcv_write (packet, outBuf, 30 - 2);
    tcv_endp (packet);

  } transient SENT: {
    ser_outf (SENT, "sent: %s\n", outBuf + 2);
    delay (2048, SENDSIGNAL);
    release;
}}
#undef packet

#define packet (((__NT_hunter*)TheStation)->__vattr_receiver_packet)
# 39 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"
receiver_hunter::perform { _pp_enter_ (); 

# 39 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"

  
  transient RECEIVING: {
    packet = tcv_rnp (RECEIVING, fd);
    tcv_read (packet, inBuf, 30);
    tcv_endp (packet);

  } transient RECEIVED: {
    if (strncmp(inBuf + 2, "T", 1) == 0) {
      ser_out (RECEIVED, "received signal from treasure\n");
      do { ( (((PicOSNode*)TheStation)->ledsm == 0) ? _no_module_ ("LEDS", "leds") : 1 ); ((PicOSNode*)TheStation)->ledsm->leds_op (1, 2); } while (0);
    }

    proceed RECEIVING;
}}
#undef packet

# 55 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"
root_hunter::perform { _pp_enter_ (); 

# 55 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_hunter.cc"

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

    finish;
}}

void __NT_hunter::__praxis_starter () { (TheNode->tally_in_pcs()?(create root_hunter )->_pp_apid_():0); }
