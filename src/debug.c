#ifdef __GNUC__
#include <execinfo.h>
#define __USE_GNU
#include <dlfcn.h>
#include <stddef.h>

#ifdef __KERNEL__
#	include<linux/printk.h>
#	define INTERNAL_PRINT printk
#	define STD_PREFIX KERN_INFO
#	define CONT_PREFIX KERN_CONT
#else
#	define INTERNAL_PRINT printf
#	define STD_PREFIX 
#	define CONT_PREFIX
#endif // __KERNEL__

const char *getCaller(void) {
    void *callstack[4];
    const int maxFrames = sizeof(callstack) / sizeof(callstack[0]);

    Dl_info info;

    backtrace(callstack, maxFrames);

    if (dladdr(callstack[3], &info) && info.dli_sname != NULL) {
        return info.dli_sname;
    } else {
        return "<?>";
    }
}
#else
const char *getCaller(void) {
	return "<unimplemented>";
}
#endif // __GNUC__

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

const char *escape_sequence[256] = {
	['\a'] = "\\a",
	['\b'] = "\\b",
	['\f'] = "\\f",
	['\n'] = "\\n",
	['\r'] = "\\r",
	['\t'] = "\\t",
	['\v'] = "\\v",
	['\\'] = "\\\\",
	['\''] = "\\'",
	['\"'] = "\\\"",
	['\?'] = "\\?",
	['\0'] = "\\0",	
	[128 ... 255] = "??",	
};

size_t strlen_probe(char *str) {
	size_t usuable_memory = malloc_usable_size(str);
	INTERNAL_PRINT(STD_PREFIX "the usuable size is %ld\n", usuable_memory);
	size_t iter = 0;
	while(str[iter]) {
		if(str[iter] == '\n') {
			INTERNAL_PRINT(CONT_PREFIX "strlen_probe: charcter \\n, index: %ld\n", iter);
		} else {
			INTERNAL_PRINT(CONT_PREFIX "strlen_probe: charcter %c (%u), index: %ld\n", str[iter], (unsigned char)str[iter], iter);
		}
		iter++;
	}
	return iter;
}

void dbgmem(void *ptr) {
    if (ptr == NULL) {
        INTERNAL_PRINT(STD_PREFIX "pointer is NULL\n");
        return;
    }

    size_t length = malloc_usable_size(ptr);
    unsigned char *p = (unsigned char *)ptr;
	INTERNAL_PRINT(STD_PREFIX "%p points to a usable region of size %ld \n[%p - %p]\n", 
		ptr, length, ptr, ptr + length - 1);

    INTERNAL_PRINT(CONT_PREFIX "---------------------------------------------------------------\n");

    for (size_t i = 0; i < length; i++) {
		if(i % 16 == 0) {
			INTERNAL_PRINT(CONT_PREFIX "%p: ", p + i);
		}
        INTERNAL_PRINT(CONT_PREFIX "%02X ", p[i]);
        if ((i + 1) % 16 == 0) {
            INTERNAL_PRINT(CONT_PREFIX "\n");
        }
    }
    INTERNAL_PRINT(CONT_PREFIX "\n");
    
    INTERNAL_PRINT(CONT_PREFIX "---------------------------------------------------------------\n");

    for (size_t i = 0; i < length; i++) {
		if(i % 16 == 0) {
			INTERNAL_PRINT(CONT_PREFIX "%p: ", p + i);
		}
		if(escape_sequence[p[i]] == NULL) {
        	INTERNAL_PRINT(CONT_PREFIX "%c  ", p[i]);
		} else {
			INTERNAL_PRINT(CONT_PREFIX "%s ", escape_sequence[p[i]]);
		}
        if ((i + 1) % 16 == 0) {
            INTERNAL_PRINT(CONT_PREFIX "\n");
        }
    }
    INTERNAL_PRINT(CONT_PREFIX "\n");
}

