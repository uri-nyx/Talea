const std = @import("std");

pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});
    
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "TaleaZ",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    var flags = [1][]const u8{""};

    exe.linkLibC();
    // Mac os specific libs to compile with minifb
    if (target.result.isDarwin()) {
        exe.addIncludePath(.{ .path = "/usr/local/include" });
        exe.addIncludePath(.{ .path = "./src/include" });
        exe.addLibraryPath(.{ .path = "/usr/local/lib" });
        exe.addObjectFile(.{ .path = "/usr/local/lib/libminifb.a" });
        exe.linkFramework("Metal");
        exe.linkFramework("MetalKit");
        exe.linkFramework("QuartzCore");
        exe.linkFramework("Cocoa");
    } else if (target.result.os.isAtLeast(std.Target.Os.Tag.linux, std.SemanticVersion.parse("4.0.0") catch unreachable).?) {
        std.debug.print("Is this Linux? {}", .{target.result});
        exe.addIncludePath(.{ .path = "/usr/local/include" });
        exe.addIncludePath(.{ .path = "./src/include" });
        exe.addLibraryPath(.{ .path = "/usr/local/lib" });
        exe.addObjectFile(.{ .path = "/usr/local/lib/libminifb.a" });
        exe.linkSystemLibrary("X11");
        exe.linkSystemLibrary("GL");
        exe.linkSystemLibrary("GLX");
    } else if (target.result.os.isAtLeast(std.Target.Os.Tag.windows, std.Target.Os.WindowsVersion.win10).?) {
        exe.addLibraryPath(.{ .path = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.37.32822/lib/x64" });
        exe.addLibraryPath(.{ .path = "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/ucrt/x64"});
        exe.addIncludePath(.{ .path = "./src/include" });
        exe.addIncludePath(.{ .path = "C:/Program Files (x86)/MiniFB/include" });
        exe.addIncludePath(.{ .path = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.37.32822/include/GL"});
        exe.addIncludePath(.{ .path = "C:/Program Files (x86)/Windows Kits/10/Include/"});
        //exe.linkSystemLibrary("ucrt");
        exe.linkSystemLibrary("gdi32");
        exe.linkSystemLibrary("Opengl32");
        exe.linkSystemLibrary("winmm");
        exe.addObjectFile(.{ .path = "C:/Program Files (x86)/MiniFB/lib/minifb.lib" });
        //flags[0] = "-D_NO_CRT_STDIO_INLINE";
    }

    exe.addCSourceFile(.{ .file = .{ .path = "src/peripherals/video.c" }, .flags = &flags });
    const network = b.dependency("network", .{}).module("network");
    const clap = b.dependency("clap", .{}).module("clap");
    const arch = b.addModule("arch", .{ .root_source_file = .{ .path = "src/arch.zig" } });
    const memory = b.addModule("memory", .{ .root_source_file = .{ .path = "src/memory.zig" } });

    exe.root_module.addImport("network", network);
    exe.root_module.addImport("clap", clap);
    exe.root_module.addImport("arch", arch);
    exe.root_module.addImport("memory", memory);
    
    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
