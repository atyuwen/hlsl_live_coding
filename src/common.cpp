#include "common.hpp"

void MessageBoxf(tstring format, ...)
{
	va_list args;
	TCHAR msg[1024];
	va_start(args, format);
	wvsprintf(msg, format.c_str(), args);
	va_end(args);
	MessageBox(NULL, msg, TEXT("debug"), MB_OK);
}
