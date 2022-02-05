extern void enablefpu(void);
extern void setup_system_clock(void);
extern void start_timer(void);

extern void setup_usart(void);
extern void wrstr(char *s);
extern void wrchr(int value);
extern void wrdec(int value);
extern void wrhex(int value);
extern void newline(void);
