#include "sysio.h"
#include "board.h"
#include "board.cc"
void __build__hunternode (data_no_t*);
void __build__treasurenode (data_no_t*);
process Root : BoardRoot {
void buildNode (const char *tp, data_no_t *nddata) {
if (tp == NULL)  excptn ("Illegal unnamed node type in data file"); else if (tp != NULL && strcmp (tp, "hunter") == 0) __build__hunternode (nddata); else if (tp != NULL && strcmp (tp, "treasure") == 0) __build__treasurenode (nddata); else excptn ("Illegal node type in data file: %s", tp);
};
};
