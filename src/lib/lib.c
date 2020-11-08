#include <lib/lib.h>
#include <lib/fs.h>

#include <resources/Font.h>

void LIB_Initialize(){
	FS_Initialize();
	FONT_Initialize();
}
