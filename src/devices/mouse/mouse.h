#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"
#include "logging.h"

#ifndef DO_NOT_INCLUDE_RAYLIB
// Due to confilcts with the WinAPI
#include "raylib.h"
#else
// definitions we need from raylib
#include "need_from_raylib.h"
#endif

struct TaleaMachine;

/*----------------------------------------------------------------------------*/
/* DEVICE MAGIC ID                                                            */
/*----------------------------------------------------------------------------*/

#define DEVICE_MOUSE_MAGIC (1297044819) // 'MOUS'

/*----------------------------------------------------------------------------*/
/* DEVICE PORTS                                                               */
/*----------------------------------------------------------------------------*/

#define P_MOUSE_CSR          0x0 // MOUSE CONTROL STATUS REGISTER
#define P_MOUSE_X            0x1 // MOUSE pointer X position in px, high byte
#define P_MOUSE_XL           0x2 // MOUSE pointer X position in px, low byte
#define P_MOUSE_Y            0x3 // MOUSE pointer Y position in px, high byte
#define P_MOUSE_YL           0x4 // MOUSE pointer Y position in px, low byte
#define P_MOUSE_HOTSPOT      0x5 // MOUSE pointer hotspot pixel, 4bits per axis
#define P_MOUSE_BORDER_COLOR 0x6 // MOUSE custom sprite border color (bit 1)
#define P_MOUSE_FILL_COLOR   0x7 // MOUSE custom sprite fill color (bit 2)
#define P_MOUSE_ACCENT_COLOR 0x8 // MOUSE custom sprite accent color (bit 3)
#define P_MOUSE_SPRITE       0x9 // MOUSE custom sprite, pointer to DATA memory
#define P_MOUSE_SPRITEL      0xa // MOUSE custom sprite, pointer, low byte

/*----------------------------------------------------------------------------*/
/* DEVICE CONSTANTS                                                           */
/* ---------------------------------------------------------------------------*/

#define MOUSE_SPRITE_DEFAULT_ADDR 0x110

enum MouseCsr {
    MOUSE_BUTT_RIGHT = 1 << 0,
    MOUSE_BUTT_LEFT  = 1 << 1,
    MOUSE_CUSTOM     = 1 << 5,
    MOUSE_VISIBLE    = 1 << 6,
    MOUSE_IE         = 1 << 7, // MOUSE interrupt enable
};

/*----------------------------------------------------------------------------*/
/* DEVICE STRUCTURES                                                          */
/*--------------------------------------------------------------------------- */

// NONE

/*----------------------------------------------------------------------------*/
/* DEVICE State                                                           */
/*----------------------------------------------------------------------------*/

typedef struct DeviceMouse {
    u8        csr;
    bool      visible;
    bool      custom;
    bool      spriteLoaded;
    bool      renderCustomCursor;
    u8        hotspot;
    u16       x, y;
    u16       spriteAddr;
    u8        borderColor, fillColor, accentColor;
    Vector2   renderPos;
    Color     pixels[256];
    Texture2D spriteTexture;
} DeviceMouse;

/*----------------------------------------------------------------------------*/
/* DEVICE INTERFACE                                                           */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream
// Here put the callbacks you want raylib to execute on the AudioStream thread

// DEVICE INITIALIZATION
void Mouse_Reset(struct TaleaMachine *m, bool isRestart);

// DEVICE UPDATE (on vblank)
// NONE //TODO: handled by Frontend currently... not very good

void Mouse_ProcessButtonPress(struct TaleaMachine *m, int buttons, int scaledX, int scaledY);
void Mouse_UpdateCoordinates(struct TaleaMachine *m, int buttons, int scaledX, int scaledY);

// DEVICE UPDATE (on cpu tick)
// NONE

// DEVICE RENDER

// DEVICE PORT IO HANDLERS
void Mouse_Write(struct TaleaMachine *m, u8 port, u8 value);
u8   Mouse_Read(struct TaleaMachine *m, u8 port);

#endif MOUSE_H