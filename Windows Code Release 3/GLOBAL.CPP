#include <Windows.h>
#include "global.h"
#include "math.h"
#include <stdlib.h>

extern short give_delays;

void alter_rect(RECT *r) 
{
	short a;

	a = r->top;
	r->top = r->left;
	r->left = a;
	a = r->bottom;
	r->bottom = r->right;
	r->right = a;
}
short get_ran (short times,short  min,short  max)
{
	short store;
	short i, to_ret = 0;

	if ((max - min + 1) == 0)
		return 0;
	for (i = 1; i < times + 1; i++) {
		store = rand() % (max - min + 1);
		to_ret = to_ret + min + store;
		}
	return to_ret;
}

Boolean same_point(location p1,location p2)
{
	if ((p1.x == p2.x) & (p1.y == p2.y))
		return TRUE;
		else return FALSE;
}

short move_to_zero(short val)
{
	if (val < 0)
		return val + 1;
	if (val > 0)
		return val - 1;
	return val;
}

short max(short a,short b)
{
	if (a > b)
		return a;
		else return b;
}

short min(short a,short b)
{
	if (a < b)
		return a;
		else return b;
}

short minmax(short min,short max,short k)
{
	if (k < min)
		return min;
	if (k > max)
		return max;
	return k;
}

short s_pow(short x,short y)
{
	if (y == 0)
   	return 1;
	return (short) pow((double) x, (double) y);
}

short a_v(short x)
{
	if (x < 0)
		return (-1 * x);
		else return x;
}
short ex_abs(short x)
{
	if (x < 0)
		return (-1 * x);
		else return x;
}

void Delay(short val,long *dummy)
{
	long then,now,wait_val;

	wait_val = (long) val;
	wait_val = wait_val * 16;
	then = (long)GetCurrentTime();
	now = then;
	while (now - then < wait_val)   {
		now = (long) GetCurrentTime();
		}
}

void pause(short length)
{
	long dummy,len;
	
	len = (long) length;
	
	if (give_delays == 0)
		Delay(len, &dummy);
}

// stuff done legit, i.e. flags are within proper ranges for stuff done flag
Boolean sd_legit(short a, short b)
{
	if ((minmax(0,299,a) == a) && (minmax(0,9,b) == b))
		return TRUE;
	return FALSE;
}
