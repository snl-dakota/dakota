#include <windows.h>
#include <wincon.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
void WINAPI plConsole(int Create);
#ifdef __cplusplus
}
#endif


void WINAPI plConsole(int Create) {

   int hCrt,i;
   FILE *hf;

   if (Create != 0) {
		AllocConsole();
		hCrt = _open_osfhandle(
             (long) GetStdHandle(STD_OUTPUT_HANDLE),
             _O_TEXT);
		hf = _fdopen( hCrt, "w" );
		*stdout = *hf;
		i = setvbuf( stdout, NULL, _IONBF, 0 );
	}
	else { 
		FreeConsole();
	}
}
