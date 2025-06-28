#ifndef BOOLEAN_T
#define BOOLEAN_T int
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef UI32_T
#define UI32_T unsigned long
#endif

#ifndef UI16_T
#define UI16_T unsigned short
#endif

#ifndef UI8_T
#define UI8_T unsigned char
#endif

#ifndef I32_T
#define I32_T unsigned long
#endif

#ifndef LPARAM
#define LPARAM unsigned long
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

#ifndef DISABLED
#define DISABLED 2
#define ENABLED  3
#endif

#ifdef HPF
#undef HPF
#endif
#define HPF	hprintf
extern char* hprintf(char *format, ...);


int	fWeb_Language_Get(void);
int	fWeb_Language_Set(int language);

