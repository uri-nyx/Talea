#include "core/bus.h"
#include "string.h"
#include "talea.h"

#define P_MOUSE_CSR          (DEV_MOUSE_BASE + 0)
#define P_MOUSE_X            (DEV_MOUSE_BASE + 1)
#define P_MOUSE_Y            (DEV_MOUSE_BASE + 3)
#define P_MOUSE_HOTSPOT      (DEV_MOUSE_BASE + 5)
#define P_MOUSE_BORDER_COLOR (DEV_MOUSE_BASE + 6)
#define P_MOUSE_FILL_COLOR   (DEV_MOUSE_BASE + 7)
#define P_MOUSE_ACCENT_COLOR (DEV_MOUSE_BASE + 8)
#define P_MOUSE_SPRITE       (DEV_MOUSE_BASE + 9)

#define ASSEMBLE_CSR(mouse) \
    (mouse->csr | (mouse->visible ? MOUSE_VISIBLE : 0) | (mouse->custom ? MOUSE_CUSTOM : 0))

void Mouse_ProcessButtonPress(TaleaMachine *m, int buttons, int scaled_x, int scaled_y)
{
    Mouse_UpdateCoordinates(m, buttons, scaled_x, scaled_y);
    if (m->mouse.csr & MOUSE_IE)
        Machine_RaiseInterrupt(m, INT_MOUSE_PRESSED, PRIORITY_KEYBOARD_INTERRUPT);
}

void Mouse_UpdateCoordinates(TaleaMachine *m, int buttons, int scaled_x, int scaled_y)
{
    DeviceMouse *mouse = &m->mouse;

    mouse->csr &= ~(MOUSE_BUTT_LEFT | MOUSE_BUTT_RIGHT);
    mouse->csr |= buttons;
    mouse->x = scaled_x;
    mouse->y = scaled_y;
}

u8 Mouse_ReadHandler(TaleaMachine *m, u16 addr)
{
    DeviceMouse *mouse = &m->mouse;

    switch (addr) {
    case P_MOUSE_CSR: return ASSEMBLE_CSR(mouse);
    case P_MOUSE_X: return mouse->x >> 8;
    case P_MOUSE_X + 1: return mouse->x;
    case P_MOUSE_Y: return mouse->y >> 8;
    case P_MOUSE_Y + 1: return mouse->y;
    case P_MOUSE_HOTSPOT: return mouse->hotspot;
    case P_MOUSE_BORDER_COLOR: return mouse->border_color;
    case P_MOUSE_FILL_COLOR: return mouse->fill_color;
    case P_MOUSE_ACCENT_COLOR: return mouse->accent_color;
    case P_MOUSE_SPRITE: return mouse->sprite_addr >> 8;
    case P_MOUSE_SPRITE + 1: return mouse->sprite_addr;
    default: return 0;
    }
}

static inline void Mouse_LoadSprite(TaleaMachine *m)
{
    DeviceMouse *mouse                     = &m->mouse;
    u8           sprite2bit[(16 * 16) / 4] = { 0 };

    TALEA_LOG_TRACE("Loading cursor sprite\n");
    if (mouse->sprite_addr + sizeof(sprite2bit) < TALEA_DATA_MEM_SZ) {
        memcpy(sprite2bit, &m->data_memory[mouse->sprite_addr], sizeof(sprite2bit));
    }

    for (size_t i = 0; i < 16 * 16; i++) {
        u8 packed = sprite2bit[i / 4];
        u8 bits   = (packed >> (6 - ((i % 4) * 2))) & 0x03;

        Color pixel_color = BLANK;
        switch (bits) {
        case 0: pixel_color = BLANK; break;
        case 1: {
            size_t index = mouse->border_color * 4;
            pixel_color  = (Color){
                 .r = (u8)m->video.renderer.shaderPalette[index],
                 .g = (u8)m->video.renderer.shaderPalette[index + 1],
                 .b = (u8)m->video.renderer.shaderPalette[index + 2],
                 .a = (u8)m->video.renderer.shaderPalette[index + 3],
            };

            break;
        }
        case 2: {
            size_t index = mouse->fill_color * 4;
            pixel_color  = (Color){
                 .r = (u8)m->video.renderer.shaderPalette[index],
                 .g = (u8)m->video.renderer.shaderPalette[index + 1],
                 .b = (u8)m->video.renderer.shaderPalette[index + 2],
                 .a = (u8)m->video.renderer.shaderPalette[index + 3],
            };

            break;
        }
        case 3: {
            size_t index = mouse->accent_color * 4;
            pixel_color  = (Color){
                 .r = (u8)m->video.renderer.shaderPalette[index],
                 .g = (u8)m->video.renderer.shaderPalette[index + 1],
                 .b = (u8)m->video.renderer.shaderPalette[index + 2],
                 .a = (u8)m->video.renderer.shaderPalette[index + 3],
            };

            break;
        }
        default: break;
        }

        mouse->pixels[i] = pixel_color;
    }

    UpdateTexture(mouse->sprite_texture, mouse->pixels);
}

void Mouse_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    DeviceMouse *mouse = &m->mouse;
    switch (addr) {
    case P_MOUSE_CSR:
        mouse->csr     = value;
        mouse->visible = value & MOUSE_VISIBLE;
        mouse->custom  = value & MOUSE_CUSTOM;

        if (mouse->visible) {
            if (mouse->custom) {
                HideCursor();

                if (!mouse->sprite_loaded) {
                    Mouse_LoadSprite(m);
                    mouse->sprite_loaded = true;
                }
                mouse->render_custom_cursor = true;
            } else {
                ShowCursor();
            }
        } else {
            HideCursor();
        }

        break;
    case P_MOUSE_HOTSPOT: mouse->hotspot = value; break;
    case P_MOUSE_BORDER_COLOR: mouse->border_color = value; break;
    case P_MOUSE_FILL_COLOR: mouse->fill_color = value; break;
    case P_MOUSE_ACCENT_COLOR: mouse->accent_color = value; break;
    case P_MOUSE_SPRITE:
        mouse->sprite_addr = (mouse->sprite_addr & 0x00ff) | (value << 8);
        // TALEA_LOG_TRACE("Written value %02x to addr %01x\n", mouse->sprite_addr >> 8, addr);
        break;
    case P_MOUSE_SPRITE + 1:
        mouse->sprite_addr = (mouse->sprite_addr & 0xff00) | (value);
        // TALEA_LOG_TRACE("Written value %02x to addr %01x\n", mouse->sprite_addr, addr);
        break;
    case P_MOUSE_X:
    case P_MOUSE_X + 1:
    case P_MOUSE_Y:
    case P_MOUSE_Y + 1:
    default: break;
    }
}

void Mouse_Reset(TaleaMachine *m, bool is_restart)
{
    DeviceMouse *mouse = &m->mouse;

    mouse->border_color = 0x255;
    mouse->fill_color   = 0xf;
    mouse->accent_color = 37;
    mouse->hotspot      = 0;
    mouse->visible      = true;
    mouse->custom       = false;
    mouse->csr          = 0;
    mouse->csr          = ASSEMBLE_CSR(mouse);
    mouse->sprite_addr  = TALEA_MOUSE_SPRITE_ADDR;

    memset(mouse->pixels, 0, sizeof(mouse->pixels));

    if (!is_restart) {
        Image sprite = (Image){
            .data    = mouse->pixels,                     // Image raw data
            .width   = 16,                                // Image base width
            .height  = 16,                                // Image base height
            .mipmaps = 1,                                 // Mipmap levels, 1 by default
            .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, // Data format (PixelFormat type)
        };
        mouse->sprite_texture = LoadTextureFromImage(sprite);
    }
}