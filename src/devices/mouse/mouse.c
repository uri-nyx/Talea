#include "mouse.h"
#include "talea.h"
#include "core/bus.h"

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

// Put statics here

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

#define ASSEMBLE_CSR(mouse) \
    (mouse->csr | (mouse->visible ? MOUSE_VISIBLE : 0) | (mouse->custom ? MOUSE_CUSTOM : 0))

static inline loadSprite(TaleaMachine *m)
{
    DeviceMouse *mouse                     = &m->mouse;
    u8           sprite2bit[(16 * 16) / 4] = { 0 };

    TALEA_LOG_TRACE("Loading cursor sprite\n");
    if (mouse->spriteAddr + sizeof(sprite2bit) < TALEA_DATA_MEM_SZ) {
        u8 *spritePtr = Bus_GetDataPointer(m, mouse->spriteAddr, sizeof(sprite2bit));
        if (!spritePtr)
            TALEA_LOG_WARNING("Cursor sprite could not be loaded: DATA pointer request denied\n");
        else
            memcpy(sprite2bit, spritePtr, sizeof(sprite2bit));
    }

    for (size_t i = 0; i < 16 * 16; i++) {
        u8 packed = sprite2bit[i / 4];
        u8 bits   = (packed >> (6 - ((i % 4) * 2))) & 0x03;

        Color color = BLANK;
        switch (bits) {
        case 0: color = BLANK; break;
        case 1: {
            size_t index = mouse->borderColor * 4;
            color        = (Color){
                       .r = (u8)m->video.renderer.shaderPalette[index],
                       .g = (u8)m->video.renderer.shaderPalette[index + 1],
                       .b = (u8)m->video.renderer.shaderPalette[index + 2],
                       .a = (u8)m->video.renderer.shaderPalette[index + 3],
            };

            break;
        }
        case 2: {
            size_t index = mouse->fillColor * 4;
            color        = (Color){
                       .r = (u8)m->video.renderer.shaderPalette[index],
                       .g = (u8)m->video.renderer.shaderPalette[index + 1],
                       .b = (u8)m->video.renderer.shaderPalette[index + 2],
                       .a = (u8)m->video.renderer.shaderPalette[index + 3],
            };

            break;
        }
        case 3: {
            size_t index = mouse->accentColor * 4;
            color        = (Color){
                       .r = (u8)m->video.renderer.shaderPalette[index],
                       .g = (u8)m->video.renderer.shaderPalette[index + 1],
                       .b = (u8)m->video.renderer.shaderPalette[index + 2],
                       .a = (u8)m->video.renderer.shaderPalette[index + 3],
            };

            break;
        }
        default: break;
        }

        mouse->pixels[i] = color;
    }

    UpdateTexture(mouse->spriteTexture, mouse->pixels);
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream

// DEVICE INITIALIZATION

void Mouse_Reset(TaleaMachine *m, bool isRestart)
{
    DeviceMouse *mouse = &m->mouse;

    mouse->borderColor = 0x255;
    mouse->fillColor   = 0xf;
    mouse->accentColor = 37;
    mouse->hotspot     = 0;
    mouse->visible     = true;
    mouse->custom      = false;
    mouse->csr         = 0;
    mouse->csr         = ASSEMBLE_CSR(mouse);
    mouse->spriteAddr  = MOUSE_SPRITE_DEFAULT_ADDR;

    memset(mouse->pixels, 0, sizeof(mouse->pixels));

    if (!isRestart) {
        Image sprite = (Image){
            .data    = mouse->pixels,                     // Image raw data
            .width   = 16,                                // Image base width
            .height  = 16,                                // Image base height
            .mipmaps = 1,                                 // Mipmap levels, 1 by default
            .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, // Data format (PixelFormat type)
        };
        mouse->spriteTexture = LoadTextureFromImage(sprite);
    }
}

// DEVICE UPDATE (on vblank)

void Mouse_ProcessButtonPress(TaleaMachine *m, int buttons, int scaledX, int scaledY)
{
    Mouse_UpdateCoordinates(m, buttons, scaledX, scaledY);
    if (m->mouse.csr & MOUSE_IE)
        Machine_RaiseInterrupt(m, INT_MOUSE_PRESSED, PRIORITY_KEYBOARD_INTERRUPT);
}

void Mouse_UpdateCoordinates(TaleaMachine *m, int buttons, int scaledX, int scaledY)
{
    DeviceMouse *mouse = &m->mouse;

    mouse->csr &= ~(MOUSE_BUTT_LEFT | MOUSE_BUTT_RIGHT);
    mouse->csr |= buttons;
    mouse->x = scaledX;
    mouse->y = scaledY;
}


// DEVICE UPDATE (on cpu tick)
// NONE

// DEVICE PORT IO HANDLERS

u8 Mouse_Read(TaleaMachine *m, u8 port)
{
    DeviceMouse *mouse = &m->mouse;

    switch (port & 0xf) {
    case P_MOUSE_CSR: return ASSEMBLE_CSR(mouse);
    case P_MOUSE_X: return mouse->x >> 8;
    case P_MOUSE_XL: return mouse->x;
    case P_MOUSE_Y: return mouse->y >> 8;
    case P_MOUSE_YL: return mouse->y;
    case P_MOUSE_HOTSPOT: return mouse->hotspot;
    case P_MOUSE_BORDER_COLOR: return mouse->borderColor;
    case P_MOUSE_FILL_COLOR: return mouse->fillColor;
    case P_MOUSE_ACCENT_COLOR: return mouse->accentColor;
    case P_MOUSE_SPRITE: return mouse->spriteAddr >> 8;
    case P_MOUSE_SPRITEL: return mouse->spriteAddr;
    default: return 0;
    }
}

void Mouse_Write(TaleaMachine *m, u8 port, u8 value)
{
    DeviceMouse *mouse = &m->mouse;
    switch (port & 0xf) {
    case P_MOUSE_CSR:
        mouse->csr     = value;
        mouse->visible = value & MOUSE_VISIBLE;
        mouse->custom  = value & MOUSE_CUSTOM;

        if (mouse->visible) {
            if (mouse->custom) {
                HideCursor();

                if (!mouse->spriteLoaded) {
                    loadSprite(m);
                    mouse->spriteLoaded = true;
                }
                mouse->renderCustomCursor = true;
            } else {
                ShowCursor();
            }
        } else {
            HideCursor();
        }

        break;
    case P_MOUSE_HOTSPOT: mouse->hotspot = value; break;
    case P_MOUSE_BORDER_COLOR: mouse->borderColor = value; break;
    case P_MOUSE_FILL_COLOR: mouse->fillColor = value; break;
    case P_MOUSE_ACCENT_COLOR: mouse->accentColor = value; break;
    case P_MOUSE_SPRITE:
        mouse->spriteAddr = (mouse->spriteAddr & 0x00ff) | (value << 8);
        // TALEA_LOG_TRACE("Written value %02x to addr %01x\n", mouse->spriteAddr >> 8, addr);
        break;
    case P_MOUSE_SPRITEL:
        mouse->spriteAddr = (mouse->spriteAddr & 0xff00) | (value);
        // TALEA_LOG_TRACE("Written value %02x to addr %01x\n", mouse->spriteAddr, addr);
        break;
    case P_MOUSE_X:
    case P_MOUSE_XL:
    case P_MOUSE_Y:
    case P_MOUSE_YL:
    default: break;
    }
}
