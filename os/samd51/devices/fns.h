#include "devices/samd51.h"

extern void enablefpu(void);

extern void start_timer(void);
extern void wait_ms(int);

extern void setup_usart(void);
extern void newline(void);
extern int rdch_ready(void);
extern int rdch(void);
extern int rdch_wait(void);
extern void wrch(int value);
extern void _wrhex(int n, int digits);
extern void wrhex(int value);
extern void write_dec(int value);
extern void wrstr(char *s);
extern void sercom1_rxc_intr(void);
extern void usart_serwrite(char *, int);

extern void enable_PORT(void);
extern void usb_init(void);
extern void usb_intr(void);
