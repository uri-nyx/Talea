#ifndef SGL_H
#define SGL_H

#define IN_KERNEL

/* @AKAI: 30000_GL */

#ifdef IN_KERNEL
#include "akai_def.h"
#include "libsirius/devices.h"
#endif

/* -----------------------------------------
   Basic Types
   ----------------------------------------- */

/* Physical address (GPU requires physical) */
typedef uptr sgl_phys;
typedef u8   sgl_color;

/* -----------------------------------------
   GPU ROPs and Rotations
   ----------------------------------------- */

typedef enum {
    sgl_ROP_COPY    = 0,
    sgl_ROP_AND     = VIDEO_CONFIG_ROP_AND,
    sgl_ROP_OR      = VIDEO_CONFIG_ROP_OR,
    sgl_ROP_XOR     = VIDEO_CONFIG_ROP_XOR,
    sgl_ROP_NOT     = VIDEO_CONFIG_ROP_NOT,
    sgl_ROP_TRANS   = VIDEO_CONFIG_ROP_TRANS,
    sgl_ROP_AND_NOT = VIDEO_CONFIG_ROP_AND_NOT,
    sgl_ROP_ADDS    = VIDEO_CONFIG_ROP_ADDS
} sgl_rop;

typedef enum {
    sgl_ROT_IDENT     = VIDEO_ROT_IDENT,
    sgl_ROT_FLIPH     = VIDEO_ROT_FLIPH,
    sgl_ROT_FLIPV     = VIDEO_ROT_FLIPV,
    sgl_ROT_90        = VIDEO_ROT_90,
    sgl_ROT_180       = VIDEO_ROT_180,
    sgl_ROT_270       = VIDEO_ROT_270,
    sgl_ROT_TRANS     = VIDEO_ROT_TRANS,
    sgl_ROT_ANTITRANS = VIDEO_ROT_ANTITRANS
} sgl_rotation;

/* -----------------------------------------
   Surfaces
   ----------------------------------------- */

typedef struct sglSurface {
    sgl_phys   phys_addr; /* physical address of pixel data */
    sgl_color *buff;
    u16        width;
    u16        height;
    u16        pitch; /* bytes per row */
    u8         bpp;   /* bits per pixel (always 8 for your GPU) */
    u8         id;    /* GPU descriptor index (0 = framebuffer) */
} sglSurface;

typedef struct sglContext {
    sglSurface  F;   /* hardware framebuffer*/
    sglSurface *T;   /* Target rendering surface */
    sgl_rop     ROP; /* global ROP */
    u8          id_bitset[256 / 8];
    u8          saved_csr;
} sglContext;

typedef struct sglDrawCall {
    enum {
        SGL_LINE,
        SGL_RECT,
        SGL_CIRCLE,
        SGL_TRI,
        SGL_BLIT,
        SGL_BLIT_STRETCH,
        SGL_PATTERN_FILL,
        SGL_FILL_HSPAN,
        SGL_FILL_VSPAN
    } type;

    union {
        struct {
            u8        t_id;
            sgl_color color;
            u16       x0, y0, x1, y1;
        } line;
        struct {
            u8        t_id;
            sgl_color color;
            u16       w, h, dx, dy;
        } rect;
        struct {
            u8        t_id;
            sgl_color outline;
            sgl_color interior;
            u8        mode;
            u16       xm, ym, r;
        } circle;
        struct {
            u8        t_id;
            sgl_color color;
            u16       x0, y0, x1, y1, x2, y2;
        } tri;
        struct {
            u8        t_id;
            sgl_color color;
            u16       x0, x1, y;
        } hspan;
        struct {
            u8        t_id;
            sgl_color color;
            u16       x, y0, y1;
        } vspan;
        struct {
            u8           t_id;
            sgl_color   *buff;
            u16          sw, sh, dx, dy;
            sgl_rotation rot;
        } blit;
        struct {
            u8           t_id;
            sgl_color   *buff;
            u16          sw, sh, dx, dy, dw, dh;
            sgl_rotation rot;
        } blit_stretched;
        struct {
            u8           t_id;
            sgl_color   *buff;
            u8           pw, ph, u, v;
            u16          dx, dy, dw, dh;
            sgl_rotation rot;
        } pattern;
    } c;
} sglDrawCall;

#define CLIP(v, m) ((v) = (v) < 0 ? 0 : (v) >= (m) ? (m - 1) : (v))

#ifndef IN_KERNEL
/* -----------------------------------------
   Initialization
   ----------------------------------------- */
/* false on error */
bool sgl_init(void);

void sgl_deinit(void);

void sgl_wait_vblank(void);

/* Bind framebuffer (surface 0) as default target
false on error
 */
bool sgl_set_target(sglSurface *surf);

/* -----------------------------------------
   Frame Control
   ----------------------------------------- */

void sgl_begin_frame(void); /* queues VIDEO_COMMAND_BEGIN_DRAWING */
void sgl_end_frame(void);   /* queues VIDEO_COMMAND_END_DRAWING   */

/* -----------------------------------------
   Surface Management
   ----------------------------------------- */

/* Create a GPU surface (must allocate physical memory!)
NULL on error
*/
sglSurface *sglSurface_create(u16 width, u16 height);
sglSurface *sglSurface_temp_target(sgl_color *buff, u16 width, u16 height);

/* Destroy surface (you free memory, GPU just forgets descriptor) */
void sglSurface_destroy(sglSurface *surf);

/* -----------------------------------------
   Drawing Primitives (GPU accelerated)
   ----------------------------------------- */

void sgl_clear(sgl_color color);

void sgl_draw_pixel(sgl_color color, u16 x, u16 y);
void sgl_draw_line(sgl_color color, u16 x0, u16 y0, u16 x1, u16 y1);
void sgl_draw_rect(sgl_color color, u16 x, u16 y, u16 w, u16 h);
void sgl_draw_circle(sgl_color color, u16 x, u16 y, i32 r, int filled);
void sgl_draw_triangle(sgl_color color, u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2);

/* -----------------------------------------
   Blitting
   ----------------------------------------- */
void sgl_blit(sgl_color *buff, u16 dx, u16 dy, u16 w, u16 h, sgl_rop rop, sgl_rotation rot);

void sgl_stretch_blit(sgl_color *buff, i32 sw, i32 sh, u16 dx, u16 dy, u16 dw, u16 dh,
                      sgl_rotation rot, sgl_rop rop);

void sgl_pattern_fill(sgl_color *buff, u8 pat_w, u8 pat_h, u8 u, u8 v, u16 dx, u16 dy, u16 dw,
                      u16 dh, sgl_rotation rot, sgl_rop rop);
#endif
/* @AKAI */
#endif /* SGL_H */
