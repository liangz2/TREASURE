#ifndef __vueehdr_treasure_h
#ifndef THREADNAME
#define THREADNAME(a) a ## _treasure
#endif
#include "board.h"
station __NT_treasure : PicOSNode { 
char __attr_init_origin [0];
# 14 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
int __vattr_fd;

# 15 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
int __vattr_sid;

# 16 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
int __vattr_channel;

# 17 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
int __vattr_power;

# 18 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
char *__vattr_outBuf,*__vattr_inBuf;

# 26 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
address __vattr_sender_packet;

# 40 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/cmpt364_project/app_treasure.cc"
address __vattr_receiver_packet;

char __attr_init_end [0];
void __praxis_starter ();
void reset () { PicOSNode::reset (); };
void init (); 
};
#include "stdattr.h"
process sender_treasure : _PP_ (__NT_treasure) {
states { SENDSIGNAL,SENT };
perform;
};
process receiver_treasure : _PP_ (__NT_treasure) {
states { RECEIVING,RECEIVED };
perform;
};
process root_treasure : _PP_ (__NT_treasure) {
states { INIT,STARTUP };
perform;
};
#endif
