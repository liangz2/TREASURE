#ifndef __vueehdr_hunter_h
#ifndef THREADNAME
#define THREADNAME(a) a ## _hunter
#endif
#include "board.h"
station __NT_hunter : PicOSNode { 
char __attr_init_origin [0];
# 14 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_fd;

# 15 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_sid;

# 16 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_channel;

# 17 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_blinkWait;

# 18 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_power;

# 19 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_currentSS;

# 20 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_currentLight;

# 21 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
char *__vattr_outBuf,*__vattr_inBuf;

# 44 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
address __vattr_sender_packet;

# 58 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
address __vattr_receiver_packet;

# 59 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
int __vattr_receiver_ss;

char __attr_init_end [0];
void __praxis_starter ();
void reset () { PicOSNode::reset (); };
void init (); 
};
#include "stdattr.h"
process sender_hunter : _PP_ (__NT_hunter) {
states { SENDSIGNAL,SENT };
perform;
};
process receiver_hunter : _PP_ (__NT_hunter) {
states { RECEIVING,RECEIVED,OUT_PUT };
perform;
};
process blinker_hunter : _PP_ (__NT_hunter) {
states { TURN_ON,TURN_OFF };
perform;
};
process root_hunter : _PP_ (__NT_hunter) {
states { INIT,STARTUP };
perform;
};
#endif
