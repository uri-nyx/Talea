///! Provides configuration constants and architecture specs
pub const ID: u8 = 'S'; // 'S' for Sirius
pub const VENDOR: u8 = 'T'; // 'T' for Taleä
pub const DEVICE_MAP = 0x100;
pub const DATA_SIZE = 1 << 16;
pub const MAIN_SIZE = 1 << 24;
pub const DeviceSize = 16;
pub const SectorSize = 512;
pub const SerialBufferSize = 256;
pub const ErrorColor: c_uint = 0xdeadbeef;
pub const DiskPath = "dev/disk/dsk"; //TODO: this must be an absolute path dynamically generated
pub const TpsPath = "dev/tps/"; //TODO: this must be an absolute path dynamically generated

// Video Module configurations

pub const DEVID = enum(u8) {
    VIDEO = 'V',
    STK = 'K', // Serial, timer, and Keyboard
    DRIVE = 'D',
};

pub const VideoConfig = struct {
    pub const WindowName = "The Taleä Sirius System";
    pub const MaxW: comptime_int = 640;
    pub const MaxH: comptime_int = 480;
    pub const DefaultW: usize = 640;
    pub const DefaultH: usize = 480;
    pub const FramebufferDefaultAddr: u24 = 0xe5_30_00;
    pub const FramebufferDefaultW: usize = 640;
    pub const FramebufferDefaultH: usize = 480;
    pub const CharbufferDefaultAddr: u24 = 0xe5_10_00;
};

pub const VideoFont = enum(u8) {
    default = 0,
};

pub const VideoMode = enum(u8) {
    monochromeText = 0,
    colorText,
    graphic332,
    graphic565,
    graphicRGBA,
    combinedRGBA,
};

pub const VideoDefaultPalette = [16]c_uint{
    0x00_00_00_ff, 0x99_00_00_ff, 0x00_66_00_ff, 0xcc_cc_00_ff, 0x00_00_99_ff, 0x99_00_99_ff, 0x00_4c_99_ff, 0xe0_e0_e0_ff,
    0x60_60_60_ff, 0xff_99_33_ff, 0x33_ff_33_ff, 0xff_ff_33_ff, 0x66_66_ff_ff, 0xff_33_ff_ff, 0x33_ff_ff_ff, 0xff_ff_ff_ff,
};

// CPU and ISA stuff
pub const IVTSize: usize = 1024;
pub const Exception = enum(u8) { 
    Reset = 0x00, 
    BusError = 0x02, 
    AddressError, 
    IllegalInstruction, 
    DivisionZero, 
    PrivilegeViolation, 
    PageFault, 
    AccessViolation 
};

pub const Interrupt = enum(u8) { 
    SerialIncoming = 0x0a, 
    KeyboardCharacter, 
    KeyboardScancode, 
    TPSFinished, 
    DiskFinished, 
    TimerTimeout, 
    TimerInterval, 
    VideoRefresh 
};

pub const ProcessorError = error{ 
    Reset, 
    BusError, 
    AddressError, 
    IllegalInstruction, 
    DivisionZero, 
    PrivilegeViolation, 
    PageFault, 
    AccessViolation 
};

pub const InterruptRaised = error{ 
    SerialIncoming, 
    KeyboardCharacter, 
    KeyboardScancode, 
    TPSFinished, 
    DiskFinished, 
    TimerTimeout, 
    TimerInterval, 
    VideoRefresh 
};

pub const MmuSizes = .{
    .Entry = 2,
    .Page = 4096,
    .PageTableEntries = 1024,
};

pub const PageTableSize = MmuSizes.PageTableEntries * MmuSizes.Entry;
pub const PageDirectoryEntries = (MAIN_SIZE / MmuSizes.Page) / MmuSizes.PageTableEntries;

pub const Decoder = .{
    .GROUP_SHIFHT = 29,
    .OPCODE_MASK = 0x1E00_0000,
    .OPCODE_SHIFT = 25,
    .R1_MASK = 0x01F0_0000,
    .R1_SHIFT = 20,
    .R2_MASK = 0x000F_8000,
    .R2_SHIFT = 15,
    .R3_MASK = 0x0000_7C00,
    .R3_SHIFT = 10,
    .R4_MASK = 0x0000_03E0,
    .R4_SHIFT = 5,
    .VECTOR_MASK = 0x0000_00FF,
    .IMM15_MASK = 0x0000_7FFF,
    .IMM20_MASK = 0x000F_FFFF,
};

pub const Group = enum(u3) {
    SYS = 0b000,
    MEM = 0b001,
    JU = 0b010,
    BRANCH = 0b011,
    LOAD = 0b100,
    ALUI = 0b101,
    ALUR = 0b110,
    STORE = 0b111,
};

pub const SYS = enum(u4) {
    Syscall = 0x2,
    GsReg = 0x3,
    SsReg = 0x4,
    Sysret = 0x6,
    Trace = 0x5,
    MmuToggle = 0x7,
    MmuMap = 0x8,
    MmuUnmap = 0x9,
    MmuStat = 0xa,
    MmuSetPT = 0xb,
    MmuUpdate = 0xc,
};

pub const MEM = enum(u4) {
    Copy,
    Swap,
    Fill,
    Through,
    From,
    Popb,
    Poph,
    Pop,
    Pushb,
    Pushh,
    Push,
    Save,
    Restore,
    Exch,
    Slt,
    Sltu,
};

pub const JU = enum(u4) { Jal, Lui, Auipc };

pub const BRANCH = enum(u4) {
    Beq,
    Bne,
    Blt,
    Bge,
    Bltu,
    Bgeu,
};

pub const LOAD = enum(u4) {
    Jalr = 1,
    Lb,
    Lbu,
    Lbd,
    Lbud,
    Lh,
    Lhu,
    Lhd,
    Lhud,
    Lw,
    Lwd,
};

pub const ALUI = enum(u4) {
    Muli,
    Mulih,
    Idivi,
    Addi,
    Subi,
    Ori,
    Andi,
    Xori,
    ShiRa,
    ShiRl,
    ShiLl,
    Slti,
    Sltiu,
};

pub const ALUR = enum(u4) {
    Add,
    Sub,
    Idiv,
    Mul,
    Or,
    And,
    Xor,
    Not,
    Ctz,
    Clz,
    Popcount,
    ShRa,
    ShRl,
    ShLl,
    Ror,
    Rol,
};

pub const STORE = enum(u4) {
    Sb,
    Sbd,
    Sh,
    Shd,
    Sw,
    Swd,
};

pub const Instruction = enum(u7) {
    Syscall = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.Syscall)),
    GsReg = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.GsReg)),
    SsReg = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.SsReg)),
    Sysret = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.Sysret)),
    Trace = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.Trace)),
    MmuToggle = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.MmuToggle)),
    MmuMap = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.MmuMap)),
    MmuUnmap = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.MmuUnmap)),
    MmuStat = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.MmuStat)),
    MmuUpdate = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.MmuUpdate)),
    MmuSetPT = (@as(u7, @intFromEnum(Group.SYS)) << 4 | @intFromEnum(SYS.MmuSetPT)),

    Copy = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Copy)),
    Swap = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Swap)),
    Fill = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Fill)),
    Through = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Through)),
    From = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.From)),
    Popb = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Popb)),
    Poph = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Poph)),
    Pop = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Pop)),
    Pushb = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Pushb)),
    Pushh = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Pushh)),
    Push = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Push)),
    Save = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Save)),
    Restore = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Restore)),
    Exch = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Exch)),
    Slt = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Slt)),
    Sltu = (@as(u7, @intFromEnum(Group.MEM)) << 4 | @intFromEnum(MEM.Sltu)),

    Jal = (@as(u7, @intFromEnum(Group.JU)) << 4 | @intFromEnum(JU.Jal)),
    Lui = (@as(u7, @intFromEnum(Group.JU)) << 4 | @intFromEnum(JU.Lui)),
    Auipc = (@as(u7, @intFromEnum(Group.JU)) << 4 | @intFromEnum(JU.Auipc)),

    Beq = (@as(u7, @intFromEnum(Group.BRANCH)) << 4 | @intFromEnum(BRANCH.Beq)),
    Bne = (@as(u7, @intFromEnum(Group.BRANCH)) << 4 | @intFromEnum(BRANCH.Bne)),
    Blt = (@as(u7, @intFromEnum(Group.BRANCH)) << 4 | @intFromEnum(BRANCH.Blt)),
    Bge = (@as(u7, @intFromEnum(Group.BRANCH)) << 4 | @intFromEnum(BRANCH.Bge)),
    Bltu = (@as(u7, @intFromEnum(Group.BRANCH)) << 4 | @intFromEnum(BRANCH.Bltu)),
    Bgeu = (@as(u7, @intFromEnum(Group.BRANCH)) << 4 | @intFromEnum(BRANCH.Bgeu)),

    Jalr = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Jalr)),
    Lb = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lb)),
    Lbu = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lbu)),
    Lbd = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lbd)),
    Lbud = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lbud)),
    Lh = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lh)),
    Lhu = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lhu)),
    Lhd = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lhd)),
    Lhud = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lhud)),
    Lw = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lw)),
    Lwd = (@as(u7, @intFromEnum(Group.LOAD)) << 4 | @intFromEnum(LOAD.Lwd)),

    Muli = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Muli)),
    Mulih = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Mulih)),
    Idivi = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Idivi)),
    Addi = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Addi)),
    Subi = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Subi)),
    Ori = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Ori)),
    Andi = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Andi)),
    Xori = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Xori)),
    ShiRa = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.ShiRa)),
    ShiRl = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.ShiRl)),
    ShiLl = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.ShiLl)),
    Slti = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Slti)),
    Sltiu = (@as(u7, @intFromEnum(Group.ALUI)) << 4 | @intFromEnum(ALUI.Sltiu)),

    Add = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Add)),
    Sub = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Sub)),
    Idiv = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Idiv)),
    Mul = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Mul)),
    Or = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Or)),
    And = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.And)),
    Xor = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Xor)),
    Not = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Not)),
    Ctz = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Ctz)),
    Clz = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Clz)),
    Popcount = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Popcount)),
    ShRa = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.ShRa)),
    ShRl = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.ShRl)),
    ShLl = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.ShLl)),
    Ror = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Ror)),
    Rol = (@as(u7, @intFromEnum(Group.ALUR)) << 4 | @intFromEnum(ALUR.Rol)),

    Sb = (@as(u7, @intFromEnum(Group.STORE)) << 4 | @intFromEnum(STORE.Sb)),
    Sbd = (@as(u7, @intFromEnum(Group.STORE)) << 4 | @intFromEnum(STORE.Sbd)),
    Sh = (@as(u7, @intFromEnum(Group.STORE)) << 4 | @intFromEnum(STORE.Sh)),
    Shd = (@as(u7, @intFromEnum(Group.STORE)) << 4 | @intFromEnum(STORE.Shd)),
    Sw = (@as(u7, @intFromEnum(Group.STORE)) << 4 | @intFromEnum(STORE.Sw)),
    Swd = (@as(u7, @intFromEnum(Group.STORE)) << 4 | @intFromEnum(STORE.Swd)),
};
