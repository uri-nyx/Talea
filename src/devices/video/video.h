#ifndef VIDEO_H
#define VIDEO_H

#include "frontend/config.h"
#include "logging.h"
#include "machine_description.h"
#include "types.h"
#include "core/bus.h"

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

#define DEVICE_VIDEO_MAGIC (1447642160) // 'VID0'

/*----------------------------------------------------------------------------*/
/* DEVICE PORTS                                                               */
/*----------------------------------------------------------------------------*/

#define P_VIDEO_COMMAND 0x0
#define P_VIDEO_GPU0    0x1
#define P_VIDEO_GPU1    0x2
#define P_VIDEO_GPU2    0x3
#define P_VIDEO_GPU3    0x4
#define P_VIDEO_GPU4    0x5
#define P_VIDEO_GPU5    0x6
#define P_VIDEO_GPU6    0x7
#define P_VIDEO_GPU7    0x8
#define P_VIDEO_GPU8    0x9
#define P_VIDEO_GPU9    0xa
#define P_VIDEO_GPU10   0xb
#define P_VIDEO_CUR_X   0xc
#define P_VIDEO_CUR_Y   0xd
#define P_VIDEO_CSR     0xe
#define P_VIDEO_ERROR   0xf

/*----------------------------------------------------------------------------*/
/* DEVICE CONSTANTS                                                           */
/* ---------------------------------------------------------------------------*/

#define VIDEO_FRAMEBUFFER_ADDR     0xE60000
#define VIDEO_TEXTBUFFER_ADDR      0xE51000
#define VIDEO_MAX_VERTEX_BUFFER_SZ (1 << 16)
#define VIDEO_CMD_MAX_ARGS         8
#define VIDEO_CMD_QUEUE_SIZE       64

enum VideoCommand {
    VIDEO_COMMAND_NOP,
    VIDEO_COMMAND_END_DRAWING,
    VIDEO_COMMAND_BEGIN_DRAWING,

    // Immediate commands
    VIDEO_COMMAND_SYS_INFO,
    VIDEO_COMMAND_BUFFER_INFO,

    // Queued commands
    VIDEO_COMMAND_CLEAR,
    VIDEO_COMMAND_SET_MODE,
    VIDEO_COMMAND_SET_ADDR,
    VIDEO_COMMAND_LOAD,
    VIDEO_COMMAND_BLIT,
    VIDEO_COMMAND_STRETCH_BLIT,
    VIDEO_COMMAND_PATTERN_FILL,

    // GPU commands (queued)
    VIDEO_COMMAND_DRAW_RECT,
    VIDEO_COMMAND_DRAW_LINE,
    VIDEO_COMMAND_DRAW_CIRCLE,
    VIDEO_COMMAND_DRAW_TRI,
    VIDEO_COMMAND_DRAW_BATCH,
    VIDEO_COMMAND_FILL_SPAN,
    VIDEO_COMMAND_FILL_VSPAN,

    VIDEO_COMMAND_SET_CSR, // queued for use in blits
    VIDEO_COMMAND_BIND_CTX,
    VIDEO_COMMAND_GET_MARKER,
};

enum VideoTextModeAttrib {
    VIDEO_TEXTMODE_CHAR = 0xFF000000,
    VIDEO_TEXTMODE_FG   = 0X00FF0000,
    VIDEO_TEXTMODE_BG   = 0X0000FF00,
    VIDEO_TEXTMODE_ATT  = 0X000000FF,

    VIDEO_TEXTMODE_ATT_CODEPAGE    = 0x01,
    VIDEO_TEXTMODE_ATT_ALT_FONT    = 0x02,
    VIDEO_TEXTMODE_ATT_TRANSPARENT = 0x04,
    VIDEO_TEXTMODE_ATT_OBLIQUE     = 0x08,
    VIDEO_TEXTMODE_ATT_DIM         = 0x10,
    VIDEO_TEXTMODE_ATT_UNDERLINE   = 0x20,
    VIDEO_TEXTMODE_ATT_BOLD        = 0x40,
    VIDEO_TEXTMODE_ATT_BLINK       = 0x80,
};

enum VideoCSR {
    VIDEO_VBLANK_EN    = 0X01,
    VIDEO_RESET_REGS   = 0X02,
    VIDEO_CURSOR_EN    = 0X04,
    VIDEO_CURSOR_BLINK = 0X08,
    VIDEO_ROP0         = 0X10,
    VIDEO_ROP1         = 0X20,
    VIDEO_ROP2         = 0X40,
    VIDEO_QUEUE_FULL   = 0X80,
};

enum VideoClearFlag {
    VIDEO_CLEAR_FLAG_TB = 1 << 0,
    VIDEO_CLEAR_FLAG_FB = 1 << 1,
};

enum VideoROP {
    VIDEO_CONFIG_ROP_COPY    = 0,
    VIDEO_CONFIG_ROP_AND     = (VIDEO_ROP0) >> 4,
    VIDEO_CONFIG_ROP_OR      = (VIDEO_ROP1) >> 4,
    VIDEO_CONFIG_ROP_XOR     = (VIDEO_ROP1 | VIDEO_ROP0) >> 4,
    VIDEO_CONFIG_ROP_NOT     = (VIDEO_ROP2) >> 4,
    VIDEO_CONFIG_ROP_TRANS   = (VIDEO_ROP2 | VIDEO_ROP0) >> 4,
    VIDEO_CONFIG_ROP_AND_NOT = (VIDEO_ROP2 | VIDEO_ROP1) >> 4,
    VIDEO_CONFIG_ROP_ADDS    = (VIDEO_ROP2 | VIDEO_ROP1 | VIDEO_ROP0) >> 4,
};

enum VideoSpriteRotation {
    VIDEO_ROT_IDENT,
    VIDEO_ROT_FLIPH,
    VIDEO_ROT_FLIPV,
    VIDEO_ROT_90,
    VIDEO_ROT_180,
    VIDEO_ROT_270,
    VIDEO_ROT_TRANS,
    VIDEO_ROT_ANTITRANS,
};

enum VideoBatchFlags {
    VIDEO_BATCH_TYPE0            = 1 << 0,
    VIDEO_BATCH_TYPE1            = 1 << 1,
    VIDEO_BATCH_BACKFACE_CULLING = 1 << 2,
    VIDEO_BATCH_ZSHADING         = 1 << 3,
    VIDEO_BATCH_MODIFY_TOPOLOGY  = 1 << 4,
    VIDEO_BATCH_ABSOLUTE         = 1 << 5,
    VIDEO_BATCH_PERSPECTIVE      = 1 << 6,
    VIDEO_BATCH_DEPTH_SORT       = 1 << 7,
};

enum VideoBatchType {
    VIDEO_BATCH_TYPE_POINT    = 0,
    VIDEO_BATCH_TYPE_LINE     = 1,
    VIDEO_BATCH_TYPE_TRISTRIP = 2,
    VIDEO_BATCH_TYPE_TRILIST  = 3,
};

enum VideoFontID {
    VIDEO_FONT_BASE_CP0,
    VIDEO_FONT_BASE_CP1,
    VIDEO_ALT_FONT_CP0,
    VIDEO_ALT_FONT_CP1,
    VIDEO_FONT_IDS
};

enum VideoMode {
    VIDEO_MODE_TEXT_MONO,
    VIDEO_MODE_TEXT_COLOR,
    VIDEO_MODE_GRAPHIC = 4,
    VIDEO_MODE_TEXT_AND_GRAPHIC,
    VIDEO_MODE_SMALL_FONT = 0x100,
};

enum VideoError {
    VIDEO_NO_ERROR = 0,
    VIDEO_NOT_ENOUGH_ARGS,
    VIDEO_TOO_MANY_ARGS,
    VIDEO_ERROR_QUEUE_FULL,
    VIDEO_ERROR_NO_MODE,
    VIDEO_ERROR_INVALID_BATCH_TYPE,
    VIDEO_ERROR_BIND,
    VIDEO_ERROR_DMA,
    VIDEO_ERROR_LOAD,
};

enum VideoDrawMode {
    VIDEO_DRAW_MODE_CIRCLE_OUTLINE = 0,
    VIDEO_DRAW_MODE_CIRCLE_FILL    = 0x2,
};

enum Video_VertexFlags {
    VIDEO_VX_END_OF_STRIP = 1 << 0,
    VIDEO_VX_BRIGHT       = 1 << 1,
    VIDEO_VX_HIDE         = 1 << 2,
    VIDEO_VX_MARK         = 1 << 3,
    VIDEO_VX_MARKID0      = 1 << 4,
    VIDEO_VX_MARKID1      = 1 << 5,
    VIDEO_VX_MARKID2      = 1 << 6,
    VIDEO_VX_MARKID3      = 1 << 7,
};

/*----------------------------------------------------------------------------*/
/* DEVICE STRUCTURES                                                          */
/*--------------------------------------------------------------------------- */

typedef i32 fx16;
#define FX16          16
#define TO_FX16(n)    ((fx16)((n) << FX16))
#define FROM_FX16(n)  ((int)((n) >> FX16))
#define VXI16_REPR_SZ 8

typedef struct {
    i16 x, y, z;
} i16v3;

typedef struct {
    i16 x, y;
} i16v2;

typedef struct {
    fx16 x, y, z;
} fx16v3;

typedef struct {
    i16 x, y, z;
    u8  color;
    u8  flags;
} vxi16;

typedef struct {
    fx16 x, y, z;
    u8   color;
    u8   flags;
} vxfx16;

typedef struct Videorenderer {
    Color backgroundColor, foregroundColor;

    RenderTexture2D *screenTexture;
    RenderTexture2D  charactersTexture;
    Texture2D        pixels;
    RenderTexture2D  framebuffer;
    Color           *charsFake;

    u32 shaderPalette[256 * 4];

    Shader fbShader;
    int    fbLoc;
    int    fbPaletteLoc;

    Shader shader;
    int    charsLoc;
    int    fontsLoc;
    int    paletteLoc;
    int    palettebgLoc;
    int    timeLoc;
    int    charSizeLoc;
    int    baseColorLoc;
    int    cursorCellIdxLoc;
    int    csrLoc;
    int    textureSizeLoc;
} VideoRenderer;

struct VideoCmd {
    enum VideoCommand op;

    u64 args[VIDEO_CMD_MAX_ARGS];
    u8  argc;
};

struct VideoCommandQueue {
    struct VideoCmd cmd[VIDEO_CMD_QUEUE_SIZE];
    u8              head, tail;
};

struct VideoFont {
    u8              charW, charH;
    RenderTexture2D atlas;
};

/*----------------------------------------------------------------------------*/
/* DEVICE State                                                           */
/*----------------------------------------------------------------------------*/

typedef struct DeviceVideo {
    enum VideoMode    mode;
    enum VideoError   error;
    enum VideoROP     rop;
    enum VideoCommand lastExecutedCommand;

    u8 ports[16];

    u8   csr;
    u8   transparency;
    bool isDrawing;
    bool vblankEnable;
    bool cursorEnable;
    bool cursorBlink;
    bool queueFull;

    struct VideoFont font;

    u8 colorShades[256][16];

    i16v3 vertexMarkers[16];

    struct VideoCommandQueue cmdQueue;

    struct VideoCmd currentCmd;

    struct Buff2D framebuffer;
    struct Buff2D textbuffer;
    struct Buff2D ctx[256];

    

    u16 cursorCellIndex;

    VideoRenderer renderer;
} DeviceVideo;

/*----------------------------------------------------------------------------*/
/* DEVICE INTERFACE                                                           */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream
// Here put the callbacks you want raylib to execute on the AudioStream thread

// DEVICE INITIALIZATION
void Video_Reset(struct TaleaMachine *m, TaleaConfig *config, bool isRestart);

// DEVICE DEINITIALIZATION
// NONE

// DEVICE UPDATE (on vblank)
void Video_Update(struct TaleaMachine *m);

// DEVICE UPDATE (on cpu tick)
// NONE

// DEVICE RENDER

// DEVICE PORT IO HANDLERS
u8   Video_Read(struct TaleaMachine *m, u8 port);
void Video_Write(struct TaleaMachine *m, u8 port, u8 value);

#endif
