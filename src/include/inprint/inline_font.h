#define inline_font_width 128
#define inline_font_height 64

static unsigned char inline_font_bits[] = {
0x01, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x39, 0xa5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x39, 0x18, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x39, 
0x18, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x19, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x98, 0xa5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x66, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xbf, 
0xe7, 0x99, 0x01, 0x83, 0xff, 0xe7, 0xe7, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xf8, 0x8f, 0xc3, 0x99, 0x24, 0x39, 0xff, 0xc3, 0xc3, 0xe7, 0xe7, 0xf3, 
0xff, 0xff, 0xff, 0xff, 0xe0, 0x83, 0x81, 0x99, 0x24, 0xe3, 0xff, 0x81, 0x81, 
0xe7, 0xcf, 0xf9, 0xff, 0xff, 0xff, 0xff, 0x80, 0x80, 0xe7, 0x99, 0x21, 0xc9, 
0xff, 0xe7, 0xe7, 0xe7, 0x80, 0x80, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x83, 0xe7, 
0x99, 0x27, 0xc9, 0x81, 0x81, 0xe7, 0x81, 0xcf, 0xf9, 0xff, 0xff, 0xff, 0xff, 
0xf8, 0x8f, 0x81, 0xff, 0x27, 0xe3, 0x81, 0xc3, 0xe7, 0xc3, 0xe7, 0xf3, 0xff, 
0xff, 0xff, 0xff, 0xfe, 0xbf, 0xc3, 0x99, 0x27, 0xcc, 0x81, 0xe7, 0xe7, 0xe7, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xe1, 0xff, 
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xc9, 0xc9, 
0xf3, 0xff, 0xe3, 0xf9, 0xe7, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 
0xe1, 0xc9, 0xc9, 0xc1, 0x9c, 0xc9, 0xf9, 0xf3, 0xf3, 0x99, 0xf3, 0xff, 0xff, 
0xff, 0xcf, 0xff, 0xe1, 0xc9, 0x80, 0xfc, 0xcc, 0xe3, 0xfc, 0xf9, 0xe7, 0xc3, 
0xf3, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xf3, 0xff, 0xc9, 0xe1, 0xe7, 0x91, 0xff, 
0xf9, 0xe7, 0x00, 0xc0, 0xff, 0xc0, 0xff, 0xf3, 0xff, 0xf3, 0xff, 0x80, 0xcf, 
0xf3, 0xc4, 0xff, 0xf9, 0xe7, 0xc3, 0xf3, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 
0xff, 0xc9, 0xe0, 0x99, 0xcc, 0xff, 0xf3, 0xf3, 0x99, 0xf3, 0xf3, 0xff, 0xf3, 
0xfc, 0xff, 0xf3, 0xff, 0xc9, 0xf3, 0x9c, 0x91, 0xff, 0xe7, 0xf9, 0xff, 0xff, 
0xf3, 0xff, 0xf3, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xc1, 0xf3, 0xe1, 0xe1, 0xc7, 0xc0, 
0xe3, 0xc0, 0xe1, 0xe1, 0xff, 0xff, 0xe7, 0xff, 0xf9, 0xe1, 0x9c, 0xf1, 0xcc, 
0xcc, 0xc3, 0xfc, 0xf9, 0xcc, 0xcc, 0xcc, 0xf3, 0xf3, 0xf3, 0xff, 0xf3, 0xcc, 
0x8c, 0xf3, 0xcf, 0xcf, 0xc9, 0xe0, 0xfc, 0xcf, 0xcc, 0xcc, 0xf3, 0xf3, 0xf9, 
0xc0, 0xe7, 0xcf, 0x84, 0xf3, 0xe3, 0xe3, 0xcc, 0xcf, 0xe0, 0xe7, 0xe1, 0xc1, 
0xff, 0xff, 0xfc, 0xff, 0xcf, 0xe7, 0x90, 0xf3, 0xf9, 0xcf, 0x80, 0xcf, 0xcc, 
0xf3, 0xcc, 0xcf, 0xff, 0xff, 0xf9, 0xff, 0xe7, 0xf3, 0x98, 0xf3, 0xcc, 0xcc, 
0xcf, 0xcc, 0xcc, 0xf3, 0xcc, 0xe7, 0xf3, 0xf3, 0xf3, 0xc0, 0xf3, 0xff, 0xc1, 
0xc0, 0xc0, 0xe1, 0x87, 0xe1, 0xe1, 0xf3, 0xe1, 0xf1, 0xf3, 0xf3, 0xe7, 0xff, 
0xf9, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xf9, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xf3, 0xc0, 0xc3, 0xe0, 0x80, 0x80, 0xc3, 
0xcc, 0xe1, 0x87, 0x98, 0xf0, 0x9c, 0x9c, 0xe3, 0x9c, 0xe1, 0x99, 0x99, 0xc9, 
0xb9, 0xb9, 0x99, 0xcc, 0xf3, 0xcf, 0x99, 0xf9, 0x88, 0x98, 0xc9, 0x84, 0xcc, 
0x99, 0xfc, 0x99, 0xe9, 0xe9, 0xfc, 0xcc, 0xf3, 0xcf, 0xc9, 0xf9, 0x80, 0x90, 
0x9c, 0x84, 0xcc, 0xc1, 0xfc, 0x99, 0xe1, 0xe1, 0xfc, 0xc0, 0xf3, 0xcf, 0xe1, 
0xf9, 0x80, 0x84, 0x9c, 0x84, 0xc0, 0x99, 0xfc, 0x99, 0xe9, 0xe9, 0x8c, 0xcc, 
0xf3, 0xcc, 0xc9, 0xb9, 0x94, 0x8c, 0x9c, 0xfc, 0xcc, 0x99, 0x99, 0xc9, 0xb9, 
0xf9, 0x99, 0xcc, 0xf3, 0xcc, 0x99, 0x99, 0x9c, 0x9c, 0xc9, 0xe1, 0xcc, 0xc0, 
0xc3, 0xe0, 0x80, 0xf0, 0x83, 0xcc, 0xe1, 0xe1, 0x98, 0x80, 0x9c, 0x9c, 0xe3, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xc0, 0xe1, 0xc0, 0xe1, 0xc0, 0xcc, 0xcc, 0x9c, 0x9c, 0xcc, 
0x80, 0xe1, 0xfc, 0xe1, 0xf7, 0xff, 0x99, 0xcc, 0x99, 0xcc, 0xd2, 0xcc, 0xcc, 
0x9c, 0x9c, 0xcc, 0x9c, 0xf9, 0xf9, 0xe7, 0xe3, 0xff, 0x99, 0xcc, 0x99, 0xf8, 
0xf3, 0xcc, 0xcc, 0x9c, 0xc9, 0xcc, 0xce, 0xf9, 0xf3, 0xe7, 0xc9, 0xff, 0xc1, 
0xcc, 0xc1, 0xf1, 0xf3, 0xcc, 0xcc, 0x94, 0xe3, 0xe1, 0xe7, 0xf9, 0xe7, 0xe7, 
0x9c, 0xff, 0xf9, 0xc4, 0xc9, 0xc7, 0xf3, 0xcc, 0xcc, 0x80, 0xe3, 0xf3, 0xb3, 
0xf9, 0xcf, 0xe7, 0xff, 0xff, 0xf9, 0xe1, 0x99, 0xcc, 0xf3, 0xcc, 0xe1, 0x88, 
0xc9, 0xf3, 0x99, 0xf9, 0x9f, 0xe7, 0xff, 0xff, 0xf0, 0xc7, 0x98, 0xe1, 0xe1, 
0xc0, 0xf3, 0x9c, 0x9c, 0xe1, 0x80, 0xe1, 0xbf, 0xe1, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0x00, 0xf3, 0xff, 0xf8, 0xff, 0xc7, 0xff, 0xe3, 0xff, 0xf8, 0xf3, 0xcf, 0xf8, 
0xf1, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xf9, 0xff, 0xcf, 0xff, 0xc9, 0xff, 0xf9, 
0xff, 0xff, 0xf9, 0xf3, 0xff, 0xff, 0xff, 0xe7, 0xe1, 0xf9, 0xe1, 0xcf, 0xe1, 
0xf9, 0x91, 0xc9, 0xf1, 0xcf, 0x99, 0xf3, 0xcc, 0xe0, 0xe1, 0xff, 0xcf, 0xc1, 
0xcc, 0xc1, 0xcc, 0xf0, 0xcc, 0x91, 0xf3, 0xcf, 0xc9, 0xf3, 0x80, 0xcc, 0xcc, 
0xff, 0xc1, 0x99, 0xfc, 0xcc, 0xc0, 0xf9, 0xcc, 0x99, 0xf3, 0xcf, 0xe1, 0xf3, 
0x80, 0xcc, 0xcc, 0xff, 0xcc, 0x99, 0xcc, 0xcc, 0xfc, 0xf9, 0xc1, 0x99, 0xf3, 
0xcc, 0xc9, 0xf3, 0x94, 0xcc, 0xcc, 0xff, 0x91, 0xc4, 0xe1, 0x91, 0xe1, 0xf0, 
0xcf, 0x98, 0xe1, 0xcc, 0x98, 0xe1, 0x9c, 0xcc, 0xe1, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xe7, 0xf8, 
0x91, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xf3, 0xe7, 0xf3, 0xc4, 0xf7, 0xc4, 0x91, 0xc4, 0xc1, 0xc1, 0xcc, 0xcc, 0x9c, 
0x9c, 0xcc, 0xc0, 0xf3, 0xe7, 0xf3, 0xff, 0xe3, 0x99, 0xcc, 0x91, 0xfc, 0xf3, 
0xcc, 0xcc, 0x94, 0xc9, 0xcc, 0xe6, 0xf8, 0xff, 0xc7, 0xff, 0xc9, 0x99, 0xcc, 
0x99, 0xe1, 0xf3, 0xcc, 0xcc, 0x80, 0xe3, 0xcc, 0xf3, 0xf3, 0xe7, 0xf3, 0xff, 
0x9c, 0xc1, 0xc1, 0xf9, 0xcf, 0xd3, 0xcc, 0xe1, 0x80, 0xc9, 0xc1, 0xd9, 0xf3, 
0xe7, 0xf3, 0xff, 0x9c, 0xf9, 0xcf, 0xf0, 0xe0, 0xe7, 0x91, 0xf3, 0xc9, 0x9c, 
0xcf, 0xc0, 0xc7, 0xe7, 0xf8, 0xff, 0x80, 0xf0, 0x87, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff  };