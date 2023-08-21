#include "devices/apollo3.h"

extern void enablefpu(void);

extern void start_timer(void);
extern void wait_ms(int);

extern void setup_usart(void);
extern void newline(void);
extern int rdch_ready(void);
extern int rdch(void);
extern int rdch_wait(void);
extern void wrch(int value);
extern void wrhex(int value);
extern void write_dec(int value);
extern void wrstr(char *s);
extern void uart_serwrite(char *, int);

extern void enable_PORT(void);
