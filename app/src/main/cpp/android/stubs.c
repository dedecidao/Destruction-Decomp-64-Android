/*
 * Parts of the PC host runtime that have no Android counterpart:
 * raphnet N64-to-USB adapters (Win32 HID / Linux hidraw) and the sampling
 * profiler. Their callers handle "no adapter" already.
 */
#include <stdint.h>

#include "port_raphnet.h"

void Port_RaphnetStart(void) {}
void Port_RaphnetStop(void) {}
int Port_RaphnetOnline(void) { return 0; }
int Port_RaphnetPoll(int chan, uint16_t *btn, int8_t *x, int8_t *y) { return 0; }
int Port_RaphnetPeek(int chan) { return 0; }
void Port_RaphnetRumble(int chan, int on) {}

void Port_ProfileStart(void) {}
