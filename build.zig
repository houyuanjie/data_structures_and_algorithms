const std = @import("std");

const DEFAULT_C_FLAGS: []const []const u8 = &.{
    "-std=c11",
    "-Wall",
    "-Wextra",
};

/// 递归收集指定目录下所有 .c 文件的相对路径
fn collectCSourceFiles(
    b: *std.Build,
    dir_path: []const u8,
) []const []const u8 {
    var list: std.ArrayList([]const u8) = .empty;
    defer list.deinit(b.allocator);

    var dir = b.build_root.handle.openDir(dir_path, .{ .iterate = true }) catch |err| {
        std.log.warn("无法打开目录 {s}: {}", .{ dir_path, err });
        return &[_][]const u8{};
    };
    defer dir.close();

    var walker = dir.walk(b.allocator) catch |err| {
        std.log.warn("遍历目录 {s} 失败: {}", .{ dir_path, err });
        return &[_][]const u8{};
    };
    defer walker.deinit();

    while (walker.next() catch null) |entry| {
        if (entry.kind == .file and std.mem.endsWith(u8, entry.basename, ".c")) {
            const rel_path = std.fs.path.join(b.allocator, &.{ dir_path, entry.path }) catch continue;
            list.append(b.allocator, rel_path) catch continue;
        }
    }

    return list.toOwnedSlice(b.allocator) catch |err| {
        std.log.warn("无法分配内存返回文件列表: {}", .{err});
        return &[_][]const u8{};
    };
}

/// 为 bin/ 目录下的每个 .c 文件生成可执行文件 + run-xxx step
fn addBinExecutables(
    b: *std.Build,
    lib: *std.Build.Step.Compile,
    dir_path: []const u8,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) void {
    var bin_dir = b.build_root.handle.openDir(dir_path, .{ .iterate = true }) catch |err| {
        std.log.warn("无法打开目录 {s}: {}", .{ dir_path, err });
        return;
    };
    defer bin_dir.close();

    var walker = bin_dir.walk(b.allocator) catch |err| {
        std.log.warn("遍历目录 {s} 失败: {}", .{ dir_path, err });
        return;
    };
    defer walker.deinit();

    while (walker.next() catch null) |entry| {
        if (entry.kind == .file and std.mem.endsWith(u8, entry.basename, ".c")) {
            const basename = entry.basename;
            const name = basename[0 .. basename.len - 2]; // 去掉 .c

            const rel_path = std.fs.path.join(b.allocator, &.{ dir_path, entry.path }) catch continue;
            const c_file = b.path(rel_path);

            const exe_mod = b.createModule(.{
                .target = target,
                .optimize = optimize,
                .link_libc = true,
            });
            exe_mod.addIncludePath(b.path("lib"));
            exe_mod.linkLibrary(lib);

            const exe = b.addExecutable(.{
                .name = name,
                .root_module = exe_mod,
            });

            exe_mod.addCSourceFile(.{
                .file = c_file,
                .flags = DEFAULT_C_FLAGS,
            });

            b.installArtifact(exe);

            const run_cmd = b.addRunArtifact(exe);
            if (b.args) |args| run_cmd.addArgs(args);

            const run_step_name = std.fmt.allocPrint(b.allocator, "run-{s}", .{name}) catch continue;
            const run_step_desc = std.fmt.allocPrint(
                b.allocator,
                "运行 bin/{s}.c",
                .{name},
            ) catch continue;

            const run_step = b.step(run_step_name, run_step_desc);
            run_step.dependOn(&run_cmd.step);
        }
    }
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // 1. 构建 lib/ 静态库
    const lib_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    lib_mod.addIncludePath(b.path("lib"));

    const lib = b.addLibrary(.{
        .linkage = .static,
        .name = "dsa",
        .root_module = lib_mod,
    });

    const lib_sources = collectCSourceFiles(b, "lib");
    if (lib_sources.len > 0) {
        lib_mod.addCSourceFiles(.{
            .files = lib_sources,
            .flags = DEFAULT_C_FLAGS,
        });
    }

    b.installArtifact(lib);

    // 2. 构建 bin/ 下所有 .c 可执行文件
    addBinExecutables(b, lib, "bin", target, optimize);
}
