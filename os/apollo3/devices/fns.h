#include "devices/apollo3.h"

extern void enablefpu(void);
extern void disablefpu(void);

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

extern void setup_spi(void);
extern void spi_set_frequency_divider(int divider);
extern int spi_send_byte(int b);
extern void spi_change_cs(int);
extern void spi_mask_set(int addr, int mask, int set);

extern void setup_i2c(void);

extern void UC8159_init(void);
extern void UC8159_start(void);
extern void UC8159_finish(void);
extern int UC8159_get_status(void);
extern int UC8159_send_parameter(int);
