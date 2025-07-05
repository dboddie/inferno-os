//#include "devices/picoplus2.h"

extern void enablefpu(void);
extern void disablefpu(void);

extern void setup_clocks(void);
extern void start_timer(void);
extern void wait_ms(int);

extern void setup_uart(void);
extern void newline(void);
extern int rdch_ready(void);
extern int rdch(void);
extern int rdch_wait(void);
extern void wrch(int value);
extern void wrhex(int value);
extern void write_dec(int value);
extern void wrstr(char *s);
extern void uart_serwrite(char *, int);

extern void setup_spi(void);
extern void spi_set_frequency_divider(int divider);
extern int spi_send_byte(int b);
extern void spi_change_cs(int);
extern void spi_mask_set(int addr, int mask, int set);

extern void setup_i2c(void);

extern void setup_led(void);
extern int get_led(void);
extern void set_led(int);

extern void usb_serwrite(char *, int);

extern void buttons_init(void);
extern int buttons_user(void);
