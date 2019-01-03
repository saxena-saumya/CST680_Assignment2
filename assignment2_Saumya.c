/* Author: Saumya Saxena
Assignment 2 - Computing in the Small
Date: May 15, 2018
*/

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "io.h"

int pin=27;
int ledmode =0;
int touchcount =0;

enum{
	GPLEV = 0x7e200034
};

enum{
	CMStart,
	CMStop,
	CMOn,
	CMOff,
	CMBlink
};

static Cmdtab cmds[] = {
	{CMStart, "start", 1},
	{CMStop, "stop", 1},
	{CMOn, "on", 1},
	{CMOff, "off", 1},
	{CMBlink, "blink", 1}
};

static int ifuncs[] = { Input, Output, Alt5, Alt4, Alt0, Alt1, Alt2, Alt3, -1};

static long
ssctread(Chan *c, void *buf, long n, vlong offset)
{
	char lbuf[50];
	switch(ledmode){
		case 1:
		if(touchcount%3==0){
			strcpy(lbuf, "Scanning:On, Mode:On");
		}
		else if(touchcount%3==1){
			strcpy(lbuf, "Scanning:On, Mode:Off");
		}
		else if(touchcount%3==2){
			strcpy(lbuf, "Scanning:On, Mode:Blink");
		}
		break;
		case 2:
		strcpy(lbuf, "Scanning:Off, Mode:Off");
		break;
		case 3:
		strcpy(lbuf, "Scanning:On, Mode:On");
		break;
		case 4:
		strcpy(lbuf, "Scanning:On, Mode:Off");
		break;
		case 5:
		strcpy(lbuf, "Scanning:On, Mode:Blink");
		break;
	return readstr(offset, buf, n, lbuf);
	}
}

void
ssctrand(void *)
{
	while(ledmode==5){
		gpioout(pin,1);
		tsleep(&up->sleep, return0, 0, 300);
		gpioout(pin,0);
		tsleep(&up->sleep, return0, 0, 300);
	}
}

void
ssctraw(void *)
{
	vlong prev, curr;

	prev = ((ulong *)GPLEV)[0];

	while(ledmode==1)
	{
		curr = ((ulong *)GPLEV)[0];

		if(prev!=curr){
			touchcount++;
			if(touchcount%3 == 0)
			{
				gpioout(pin, 1);
			}
			else if(touchcount%3 ==1)
			{
				gpioout(pin, 0);
			}
			else if(touchcount%3 == 2)
			{
				while(touchcount%3 == 2){
					gpioout(pin,1);
					tsleep(&up->sleep, return0, 0, 300);
					gpioout(pin,0);
					tsleep(&up->sleep, return0, 0, 300);
				}
			}
			print("Reg val: %x\n", gvals);
		}
		prev = curr;
	}
}

static long
ssctwrite(Chan *c, void *buf, long n, vlong)
{
	Cmdbuf *cb;
	Cmdtab *ct;
	int i;

	cb = parsecmd(buf, n);
	if(waserror()){
		free(cb);
		nexterror();
	}
	ct = lookupcmd(cb, cmds, nelem(cmds));
	gpiosel(pin, Output);
	switch(ct->index){
	case CMStart:
		gpioout(pin, 1);
		ledmode=1;
		kproc("ssct", ssctraw, nil);
		touchcount=0;
		break;
	case CMStop:
		gpioout(pin, 0);
		ledmode=2;
		break;
	case CMOn:
		gpioout(pin, 1);
		ledmode=3;
		break;
	case CMOff:
		gpioout(pin, 0);
		ledmode=4;
		break;
	case CMBlink:
		while(1){
			ledmode=5;
			kproc("ssct", ssctrand, nil);
		}
		break;
	}
	free(cb);
	poperror();
	return n;
}

void
ssctlink(void)
{
addarchfile("ssct",0664,ssctread,ssctwrite);
}
