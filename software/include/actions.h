#ifndef __ACTIONS__
#define __ACTIONS__


typedef void (*action)(void);

void dummy();
void up();
void down();
void select_chan();

const action actions[] = {dummy, up, down, select_chan};

#endif