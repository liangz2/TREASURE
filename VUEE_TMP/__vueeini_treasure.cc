#include "__vueehdr_treasure.h"
void __NT_treasure::init () {
memset (__attr_init_origin, 0, __attr_init_end - __attr_init_origin);
# 14 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_treasure.cc"
__vattr_fd=-1 ;
# 15 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_treasure.cc"
__vattr_sid=0 ;
# 16 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_treasure.cc"
__vattr_channel=0 ;
# 17 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_treasure.cc"
__vattr_power=0 ;
# 18 "/home/zhengyi/Documents/cmpt364/PICOS/Apps/TREASURE/app_treasure.cc"
__vattr_ss=250 ;
__praxis_starter ();
}
void __build__treasurenode (data_no_t *nddata) { create __NT_treasure (nddata); }
