#ifndef SDL2_inprint_h
#define SDL2_inprint_h

#include <SDL2/SDL.h>

extern void prepare_inline_font(void);
extern void kill_inline_font(void);

extern void inrenderer(SDL_Renderer *renderer);
extern void infont(SDL_Texture *font);
extern void incolor1(SDL_Color *color);
extern void incolor(Uint32 color, Uint32 unused); /* Color must be in 0x00RRGGBB format ! */
extern void inprint(SDL_Renderer *dst, const char *str, Uint32 x, Uint32 y);
extern void getFontwh(Uint16 *w, Uint16 *h);

extern SDL_Texture *get_inline_font(void);

#endif /* SDL2_inprint_h */
