pub const c = @cImport({
    @cInclude("MiniFB.h");
    @cDefine("STBI_ONLY_JPEG", {});
    @cDefine("STBI_ONLY_PNG", {});
    @cDefine("STBI_ONLY_BMP", {});
    @cDefine("STBI_ONLY_TGA", {});
    @cDefine("STBI_ONLY_PIC", {});
    @cDefine("STBI_ONLY_PNM", {});
    @cDefine("OLIVEC_LOADFONT", {});
    @cDefine("OLIVEC_IMPLEMENTATION", {});
    @cInclude("wrapper.h");
});
