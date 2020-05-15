#pragma once

#ifdef ASYNCFILEIO_API_EXPORTS
#define ASYNCFILEIO_API __declspec(dllexport) 
#else
#define ASYNCFILEIO_API __declspec(dllimport)
#endif

#include <stdio.h>

extern "C" ASYNCFILEIO_API bool asyncFileRead(char path[BUFSIZ], char buffer[BUFSIZ]);
extern "C" ASYNCFILEIO_API bool asyncFileWrite(char path[BUFSIZ], char buffer[BUFSIZ]);