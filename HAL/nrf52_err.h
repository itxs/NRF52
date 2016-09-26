#ifndef _ERR_H_
#define _ERR_H_

#include "stdio.h"

#ifdef DEBUG
#define LOG_ERROR(...)	//printf(...)
#endif

#ifndef DEBUG
#define LOG_ERROR(x)
#endif

#endif
