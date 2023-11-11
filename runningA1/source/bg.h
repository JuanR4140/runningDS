
//{{BLOCK(bg)

//======================================================================
//
//	bg, 256x256@8, 
//	Alphabit on.
//	+ palette 256 entries, not compressed
//	+ bitmap not compressed
//	Total size: 512 + 65536 = 66048
//
//	Time-stamp: 2023-03-17, 03:52:02
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_BG_H
#define GRIT_BG_H

#define bgBitmapLen 65536
extern const unsigned int bgBitmap[16384];

#define bgPalLen 512
extern const unsigned short bgPal[256];

#endif // GRIT_BG_H

//}}BLOCK(bg)
