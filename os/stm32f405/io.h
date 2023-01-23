/* U-Boot API definitions */

/* From api_public.h */
enum {
    API_GETC = 1,
    API_PUTC,
    API_TSTC,
    API_PUTS,
    API_RESET,
    API_DISPLAY_CLEAR = 19
};

/* From glue.h */
extern int ub_getc(void);
extern void ub_putc(char c);
extern void ub_puts(const char *s);
extern void ub_reset(void);
extern void ub_display_clear(void);
