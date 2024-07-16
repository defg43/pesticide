#ifndef DEBUG_H
#define DEBUG_H

#include "map.h"

#ifndef lengthof
#define lengthof(array) (sizeof(array) / sizeof((array)[0]))
#endif

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

#ifdef DEBUG

static void dbgmem(void *ptr);
static size_t strlen_probe(char *str);

#define free(x) \
	({ \
		dbg("freeing variable %s with the value %p", #x, x); \
		free(x); \
	})
#define dbg(fmt, ...) \
    ({  \
        INTERNAL_PRINT(STD_PREFIX "[\e[93mdebug\e[0m: %s @ %d in %s from %s] " fmt "\n", 	\
        __FUNCTION__, __LINE__, __FILE__, getCaller() __VA_OPT__(, __VA_ARGS__)); 			\
    })
#else
#define dbg(fmt, ...)
#define dbgmem(ptr) 
#endif

#ifdef DEBUG
#define _compareWithAnyofArray(arg, arr) ({											\
	bool matches = false;															\
	for(size_t i = 0; i < lengthof(arr); i++) {										\
		matches |= arg == arr[i];													\
	}																				\
	matches;																		\
})

#define _stringify_comma(x) _stringify_comma_defer(x)
#define _stringify_comma_defer(x) #x,

#define _maxFromArray(arr) ({														\
	size_t largest = 0;																\
	bool found = false;																\
	for(size_t iter = 0; iter < lengthof(arr); iter++) {							\
		if(largest < arr[iter]) {													\
			largest = arr[iter];													\
			found = true;															\
		}																			\
	}																				\
	largest;		 																\
})

#if defined(__STDC_VERSION__)
#	if __STDC_VERSION__ >= 199901L
// generic was only introduced in C11
#	define _dbgstr_convert_ptr_to_diff(x) 											\
			_Generic((x),															\
				char *: (str - x),													\
				default: (x)														\
			),
#	else
#	define _dbgstr_convert_ptr_to_diff(x) (x) // feature disabled, assumed char *
#	endif
#endif


#define dbgstr(strin, ...) ({														\
		char *str = strin; 															\
		size_t idx[] = { MAP(_dbgstr_convert_ptr_to_diff, __VA_ARGS__) };			\
		const char *varname[] = { MAP(_stringify_comma, __VA_ARGS__) };				\
        size_t len = malloc_usable_size(str); 										\
        size_t usable_size = malloc_usable_size(str);								\
		if(len == 0 && usable_size == 0 && lengthof(strin) != 1) {					\
			len = lengthof(strin); usable_size = lengthof(strin);					\
		} 																			\
        size_t index = 0;															\
        size_t arrow_index = 0;														\
        size_t arrow_pos[lengthof(idx)];											\
        bool idx_printed[lengthof(idx)] = {};										\
        size_t arrow_count = 0;														\
		size_t temp = 0;															\
																					\
		for(size_t entry = 0; entry < lengthof(idx); entry++) {						\
			size_t compensation = 0;												\
			if(idx[entry] > len) {													\
				arrow_pos[entry] = len + 2; 										\
			} else {																\
																					\
				for(size_t iterator = 0; iterator < len; iterator++) {				\
					if(iterator	== idx[entry]) {									\
						break;														\
					}																\
					if(str[iterator] == '\t' || str[iterator] == '\b' || 			\
					str[iterator] == '\r' || str[iterator] == '\e' || 				\
					str[iterator] == '\\' || str[iterator] == '\0'||				\
					str[iterator] == '\n') {										\
						compensation++;												\
					}																\
				}																	\
				arrow_pos[entry] = idx[entry] + compensation;						\
				arrow_count++;														\
				compensation = 0;													\
			}																		\
		}																			\
																					\
       	INTERNAL_PRINT(STD_PREFIX "");												\
       	while(index < usable_size) {												\
        	if(_compareWithAnyofArray(index, idx)) {								\
        		if(str[index] == ' ') {												\
        			INTERNAL_PRINT(CONT_PREFIX "\e[43m");							\
        		} else {															\
        			INTERNAL_PRINT(CONT_PREFIX "\e[33m");							\
        		}																	\
        	}																		\
        	if(str[index] == '\n') { INTERNAL_PRINT(CONT_PREFIX "\\n"); }			\
        	else if(str[index] == '\t') { INTERNAL_PRINT(CONT_PREFIX "\\t"); }		\
			else if(str[index] == '\b') { INTERNAL_PRINT(CONT_PREFIX "\\b"); }		\
			else if(str[index] == '\r') { INTERNAL_PRINT(CONT_PREFIX "\\r"); }		\
			else if(str[index] == '\e') { INTERNAL_PRINT(CONT_PREFIX "\\e"); }		\
			else if(str[index] == '\\') { INTERNAL_PRINT(CONT_PREFIX "\\"); }		\
			else if(str[index] == '\0') { INTERNAL_PRINT(CONT_PREFIX "\\0"); }		\
			else if(str[index] >= 128) { INTERNAL_PRINT(CONT_PREFIX "??"); } 		\
			else { INTERNAL_PRINT(CONT_PREFIX "%c", str[index]); }					\
			if(_compareWithAnyofArray(index, idx)) {								\
				INTERNAL_PRINT(CONT_PREFIX "\e[0m");								\
			}																		\
			index++;																\
        }																			\
        INTERNAL_PRINT(CONT_PREFIX "\n");											\
        for(int entry = 0; entry < arrow_count; entry++) {							\
       		for(size_t i = 0; i <= _maxFromArray(arrow_pos); i++) {					\
       			bool print_seperator = false;										\
	      		for(size_t check_entry = 0; check_entry < arrow_count;				\
	      		check_entry++) {													\
	      			print_seperator |= i == arrow_pos[check_entry]					\
	      				&& idx_printed[check_entry] == false;						\
	      		}																	\
      			if(print_seperator) {												\
      				INTERNAL_PRINT(CONT_PREFIX"|");									\
      			} else {															\
      				INTERNAL_PRINT(CONT_PREFIX" ");									\
      			}																	\
        	}																		\
        	INTERNAL_PRINT(CONT_PREFIX"\n");										\
																					\
       		for(size_t i = 0; i <= _maxFromArray(arrow_pos); i++) {					\
	      		if(i == arrow_pos[entry]) {											\
       				i += INTERNAL_PRINT(CONT_PREFIX									\
       					"%s [%ld/%ld]", varname[entry], idx[entry], len);			\
       				idx_printed[entry] = true;										\
	      		}																	\
       			bool print_seperator = false;										\
	      		for(size_t check_entry = 0; check_entry < arrow_count;				\
	      		check_entry++) {													\
	      			print_seperator |= i == arrow_pos[check_entry]					\
	      				&& idx_printed[check_entry] == false;						\
	      		}																	\
      			if(print_seperator) {												\
      				INTERNAL_PRINT(CONT_PREFIX "|");								\
      			} else {															\
      				INTERNAL_PRINT(CONT_PREFIX" ");									\
      			}																	\
        	}																		\
        	INTERNAL_PRINT(CONT_PREFIX "\n");										\
        }																			\
        																			\
})

#else
#define dbgstr(strin, ...)
#endif

static const char *getCaller(void);

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

static const char *getCaller(void) {
    void *callstack[4];
    const int maxFrames = sizeof(callstack) / sizeof(callstack[0]);

    Dl_info info;

    backtrace(callstack, maxFrames);

    if (dladdr(callstack[2], &info) && info.dli_sname != NULL) {
        return info.dli_sname;
    } else {
        return "<?>";
    }
}
#else
static const char *getCaller(void) {
	return "<unimplemented>";
}
#endif // __GNUC__

#include <stddef.h>
#include <stdio.h>
#ifdef __KERNEL__
#include <linux/slab.h>
#define malloc_usable_size ksize
#else 
#include <malloc.h>
#endif // not __KERNEL__
static const char *escape_sequence[256] = {
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

static void dbgmem(void *ptr) {
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

#endif // DEBUG_H
