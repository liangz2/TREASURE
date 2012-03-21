#include "__vueehdr_hunter.h"
void __NT_hunter::init () {
memset (__attr_init_origin, 0, __attr_init_end - __attr_init_origin);
# 14 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_fd=-1 ;
# 15 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_sid=0 ;
# 16 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_channel=0 ;
# 17 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_blinkWait=0 ;
# 18 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_power=0 ;
# 19 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_currentSS=0 ;
# 20 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_hunter.cc"
__vattr_currentLight=-1 ;
__praxis_starter ();
}
void __build__hunternode (data_no_t *nddata) { create __NT_hunter (nddata); }
