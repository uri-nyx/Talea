/*
    SGL, Sirius Graphics Library
*/

#include "akai.h"

#include <stdlib.h>

/* -----------------------------------------
   Initialization
   ----------------------------------------- */

/* Bind framebuffer (surface 0) as default target */
static sglContext C;

static u8 sgl_alloc_id(void)
{
    usize i;
    for (i = 1; i < 256; i++) {
        if (!BIT_TEST(C.id_bitset, i)) {
            BIT_SET(C.id_bitset, i);
            return i;
        };
    }

    return 0;
}

static void sgl_dealloc_id(u8 id)
{
    if (id) BIT_CLR(C.id_bitset, id);
}

#define ID_ALLOCATED(id) BIT_TEST(C.id_bitset, (u8)(id))

static void set_rop(sgl_rotation rop)
{
    u8 b  = rop;
    C.ROP = sgl_ROP_COPY;
    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_ROP, &b, 1);
}

bool sgl_init(void)
{
    if (ak_dev_claim(DEV_FRAMEBUFFER) != A_OK) return false;
    ak_dev_ctl(DEV_FRAMEBUFFER, DEV_RESET, NULL, 0);

    /* Set up framebuffer */
    C.F.bpp       = 8;
    C.F.buff      = (sgl_color *)AKAI_FRAMEBUFFER;
    C.F.phys_addr = ak_dev_ctl(DEV_FRAMEBUFFER, GL_GET_FRAMEBUFFER_PHYS, NULL, 0);
    C.F.width     = 640;
    C.F.height    = 480;
    C.F.pitch     = 640;
    C.F.id        = 0;

    /* Set up default target to framebuffer */
    C.T = &C.F;

    BIT_SET(C.id_bitset, 0);

    C.saved_csr = ak_dev_in(DEV_FRAMEBUFFER, VIDEO_CSR);
    ak_dev_out(DEV_FRAMEBUFFER, VIDEO_CSR, C.saved_csr | VIDEO_VBLANK_EN);

    sgl_begin_frame();
    /* Set up default ROP as copy */
    set_rop(sgl_ROP_COPY);
    sgl_clear(0);
    sgl_end_frame();

    return true;
}

void sgl_deinit(void)
{
    C.saved_csr;
    ak_dev_out(DEV_FRAMEBUFFER, VIDEO_CSR, C.saved_csr);
}

void sgl_wait_vblank(void)
{
    ak_dev_ctl(DEV_FRAMEBUFFER, VCTL_WAIT_VBLANK, NULL, 0);
}

bool sgl_set_target(sglSurface *surf)
{
    if (!surf) {
        // if NULL, set framebuffer
        C.T = &C.F;
        return true;
    }

    if (!ID_ALLOCATED(surf->id)) return false;
    C.T = surf;
    return true;
}

/* -----------------------------------------
   Frame Control
   ----------------------------------------- */

void sgl_begin_frame(void)
{
    ak_dev_out(DEV_FRAMEBUFFER, VIDEO_COMMAND, VIDEO_COMMAND_BEGIN_DRAWING);
}

void sgl_end_frame(void)
{
    ak_dev_out(DEV_FRAMEBUFFER, VIDEO_COMMAND, VIDEO_COMMAND_END_DRAWING);
}

/* -----------------------------------------
   Surface Management
   ----------------------------------------- */

/* Create a GPU surface (must allocate physical memory!) */
sglSurface *sglSurface_create(u16 width, u16 height)
{
    u8          id = sgl_alloc_id();
    sglSurface *surf;
    if (!id) return NULL;

    surf = malloc(sizeof(sglSurface));
    if (!surf) return NULL;

    surf->buff = malloc(width * height); // TODO: not enough. needs a contiguous block of page aligned memory
    if (!surf->buff) goto fail;
    memset(surf->buff, 0);

    surf->bpp    = 1;
    surf->height = height;
    surf->width  = width;
    surf->pitch  = width;
    surf->id     = id;

    surf->phys_addr = 0;
    if (ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_BIND, surf, sizeof(sglSurface)) != A_OK ||
        !surf->phys_addr) {
        free(surf->buff);
fail:
        free(surf);
        return NULL;
    }

    return surf;
}

static sglSurface tmp;
sglSurface       *sglSurface_temp_target(sgl_color *buff, u16 width, u16 height)
{
    u8          id          = sgl_alloc_id();
    sglSurface *prev_target = C.T;

    if (!id) return NULL;

    tmp.buff = buff;

    tmp.bpp    = 1;
    tmp.height = height;
    tmp.width  = width;
    tmp.pitch  = width;
    tmp.id     = id;

    tmp.phys_addr = 0;
    if (ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_BIND, &tmp, sizeof(sglSurface)) != A_OK ||
        !tmp.phys_addr) {
        return NULL;
    }

    sgl_set_target(&tmp);
    return prev_target;
}

/* Destroy surface (you free memory, GPU just forgets descriptor) */
void sglSurface_temp_untarget()
{
    sgl_dealloc_id(tmp.id); // XXX maybe checking here if the id was actually deallocated before
                             // can prevetn double frees
};

/* Destroy surface (you free memory, GPU just forgets descriptor) */
void sglSurface_destroy(sglSurface *surf)
{
    sgl_dealloc_id(surf->id); // XXX maybe checking here if the id was actually deallocated before
                              // can prevetn double frees
    free(surf->buff);
    free(surf);
};

/* -----------------------------------------
   Drawing Primitives (GPU accelerated)
   ----------------------------------------- */

void sgl_clear(sgl_color color)
{
    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CLEAR, &color, 1);
}

#define CLIP_T(x, y)       \
    CLIP((x), C.T->width); \
    CLIP((y), C.T->height)

void sgl_draw_pixel(sgl_color color, u16 x, u16 y)
{
    // clip
    CLIP_T(x, y);
    C.T->buff[y * C.T->width + x] = color;
}

void sgl_draw_line(sgl_color color, u16 x0, u16 y0, u16 x1, u16 y1)
{
    sglDrawCall c;

    c.type         = SGL_LINE;
    c.c.line.t_id  = C.T->id;
    c.c.line.color = color;
    c.c.line.x0    = x0;
    c.c.line.y0    = y0;
    c.c.line.x1    = x1;
    c.c.line.y1    = y1;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
}

void sgl_draw_rect(sgl_color color, u16 x, u16 y, u16 w, u16 h)
{
    sglDrawCall c;

    c.type         = SGL_RECT;
    c.c.rect.t_id  = C.T->id;
    c.c.rect.color = color;
    c.c.rect.dx    = x;
    c.c.rect.dy    = y;
    c.c.rect.w     = w;
    c.c.rect.h     = h;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
}

void sgl_draw_circle(sgl_color color, u16 x, u16 y, i32 r, int filled)
{
    sglDrawCall c;

    c.type              = SGL_CIRCLE;
    c.c.circle.t_id     = C.T->id;
    c.c.circle.outline  = color;
    c.c.circle.interior = color;
    c.c.circle.xm       = x;
    c.c.circle.ym       = y;
    c.c.circle.r        = r;
    c.c.circle.mode     = filled ? 0X2 : 0;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
}

void sgl_draw_triangle(sgl_color color, u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2)
{
    sglDrawCall c;

    c.type        = SGL_TRI;
    c.c.tri.t_id  = C.T->id;
    c.c.tri.color = color;
    c.c.tri.x0    = x0;
    c.c.tri.y0    = y0;
    c.c.tri.x1    = x1;
    c.c.tri.y1    = y1;
    c.c.tri.x2    = x2;
    c.c.tri.y2    = y2;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
}

/* -----------------------------------------
   Blitting
   ----------------------------------------- */

void sgl_blit(sgl_color *buff, u16 dx, u16 dy, u16 w, u16 h, sgl_rop rop, sgl_rotation rot)
{
    sglDrawCall c;

    if (rop != C.ROP) set_rop(rop);

    c.type        = SGL_BLIT;
    c.c.blit.t_id = C.T->id;
    c.c.blit.buff = buff;
    c.c.blit.dx   = dx;
    c.c.blit.dy   = dy;
    c.c.blit.sw   = w;
    c.c.blit.sh   = h;
    c.c.blit.rot  = rot;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
}

void sgl_stretch_blit(sgl_color *buff, i32 sw, i32 sh, u16 dx, u16 dy, u16 dw, u16 dh,
                      sgl_rotation rot, sgl_rop rop)
{
    sglDrawCall c;

    if (rop != C.ROP) set_rop(rop);

    c.type                  = SGL_BLIT_STRETCH;
    c.c.blit_stretched.t_id = C.T->id;
    c.c.blit_stretched.buff = buff;
    c.c.blit_stretched.dx   = dx;
    c.c.blit_stretched.dy   = dy;
    c.c.blit_stretched.sw   = sw;
    c.c.blit_stretched.sh   = sh;
    c.c.blit_stretched.dw   = dw;
    c.c.blit_stretched.dh   = dh;
    c.c.blit_stretched.rot  = rot;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
}

void sgl_pattern_fill(sgl_color *buff, u8 pat_w, u8 pat_h, u8 u, u8 v, u16 dx, u16 dy, u16 dw,
                      u16 dh, sgl_rotation rot, sgl_rop rop)
{
    sglDrawCall c;

    if (rop != C.ROP) set_rop(rop);

    c.type           = SGL_BLIT_STRETCH;
    c.c.pattern.t_id = C.T->id;
    c.c.pattern.buff = buff;
    c.c.pattern.u    = u;
    c.c.pattern.v    = v;
    c.c.pattern.pw   = pat_w;
    c.c.pattern.ph   = pat_h;
    c.c.pattern.dx   = dx;
    c.c.pattern.dy   = dy;
    c.c.pattern.dw   = dw;
    c.c.pattern.dh   = dh;
    c.c.pattern.rot  = rot;

    ak_dev_ctl(DEV_FRAMEBUFFER, GL_DO_CALL, &c, sizeof(c));
};
