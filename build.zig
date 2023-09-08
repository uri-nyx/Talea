const std = @import("std");
const deps = @import("./deps.zig");

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
    if (target.isDarwin()) {
        exe.addIncludePath(.{ .path = "/usr/local/include" });
        exe.addIncludePath(.{ .path = "./src/include" });
        exe.addLibraryPath(.{ .path = "/usr/local/lib" });
        exe.addObjectFile(.{ .path = "/usr/local/lib/libminifb.a" });
        exe.linkFramework("Metal");
        exe.linkFramework("MetalKit");
        exe.linkFramework("QuartzCore");
        exe.linkFramework("Cocoa");
    } else if (target.isLinux()) {
        exe.addIncludePath(.{ .path = "/usr/local/include" });
        exe.addIncludePath(.{ .path = "./src/include" });
        exe.addLibraryPath(.{ .path = "/usr/local/lib" });
        exe.addObjectFile(.{ .path = "/usr/local/lib/libminifb.a" });
        exe.linkSystemLibrary("X11");
        exe.linkSystemLibrary("GL");
        exe.linkSystemLibrary("GLX");
    } else if (target.isWindows()) {
        exe.addLibraryPath(.{ .path = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.37.32822/lib/x64" });
        exe.addLibraryPath(.{ .path = "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/ucrt/x64"});
        exe.addIncludePath(.{ .path = "./src/include" });
        exe.addIncludePath(.{ .path = "C:/Program Files (x86)/MiniFB/include" });
        exe.addIncludePath(.{ .path = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.37.32822/include/GL"});
        exe.addIncludePath(.{ .path = "C:/Program Files (x86)/Windows Kits/10/Include/"});
        exe.linkSystemLibrary("ucrt");
        exe.linkSystemLibrary("gdi32");
        exe.linkSystemLibrary("Opengl32");
        exe.linkSystemLibrary("winmm");
        exe.addObjectFile(.{ .path = "C:/Program Files (x86)/MiniFB/lib/minifb.lib" });
        //flags[0] = "-D_NO_CRT_STDIO_INLINE";
    }

    exe.addCSourceFile(.{ .file = .{ .path = "src/peripherals/video.c" }, .flags = &flags });

    const arch = b.addModule("arch", .{ .source_file = .{ .path = "src/arch.zig" } });
    exe.addModule("arch", arch);
    const memory = b.addModule("memory", .{ .source_file = .{ .path = "src/memory.zig" } });
    exe.addModule("memory", memory);

    deps.addAllTo(exe);

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

    // Creates a step for unit testing. This only builds the test executable
    // but does not run it.
    const unit_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    unit_tests.addModule("arch", arch);
    unit_tests.addModule("memory", memory);
    deps.addAllTo(unit_tests);

    const run_unit_tests = b.addRunArtifact(unit_tests);

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);
}
