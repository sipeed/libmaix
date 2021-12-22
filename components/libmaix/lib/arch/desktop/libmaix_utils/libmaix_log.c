
#include "stdio.h"
#include <stdarg.h>
#include <string.h>


int __android_log_print(int prio, const char *tag, const char *fmt, ...)
{
	va_list ap;
	char buf[256];
	va_start(ap, fmt);
	if(strncmp(fmt, "This application", 15)==0){
		return 0;
	}
	vsnprintf(buf, 256, fmt, ap);
	va_end(ap);
	return puts(buf);
}

