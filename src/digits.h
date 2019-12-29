#ifndef DIGITS_H
#define DIGITS_H

#include <map>

extern int zero[14][8];
extern int one[14][8];
extern int two[14][8];
extern int three[14][8];
extern int four[14][8];
extern int five[14][8];
extern int six[14][8];
extern int seven[14][8];
extern int eight[14][8];
extern int nine[14][8];
extern int divider[14][8];

extern std::map<int, int (*)[8]> digits;

#endif
