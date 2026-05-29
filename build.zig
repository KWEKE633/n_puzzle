const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const cxx_flags = &[_][]const u8{
        "-Wall",
        "-Wextra",
        "-Werror",
        "-std=c++20",
    };

    const root = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .link_libcpp = true,
    });
    root.addIncludePath(b.path("."));
    root.addCSourceFile(.{
        .file = b.path("srcs/main.cpp"),
        .flags = cxx_flags,
    });

    const exe = b.addExecutable(.{
        .name = "n_puzzle",
        .root_module = root,
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run n_puzzle");
    run_step.dependOn(&run_cmd.step);
}
