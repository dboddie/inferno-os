extern void enablefpu(void);

extern void setup_system_clock(void);
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
extern void uart3_intr(void);
extern void usart_serwrite(char *, int);

extern void setup_led(void);
extern int get_led(void);
extern void set_led(int on);
extern void toggle_led(void);

extern void enable_GPIO_A(void);
extern void enable_GPIO_B(void);
extern void enable_GPIO_C(void);

extern void setup_spi(void);
extern int spi_send_byte(int b);

extern void setup_i2c(void);

extern void UC8159_init(void);
extern void UC8159_start(void);
extern void UC8159_finish(void);
extern int UC8159_get_status(void);
extern int UC8159_send_parameter(int);
