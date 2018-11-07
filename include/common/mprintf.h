
//int u2str(char *buf, unsigned int num, int base);
int matoi(char *str);
double atod(char *str);
//int dprint(double x, int campo, int frac, void (*putc)(int));
//int va_printf(void (*putc)(int), const char *formato, va_list va);
int mprintf(void (*putc)(int), const char *formato, ... );
int msprintf(char *buf, const char *formato, ... );
