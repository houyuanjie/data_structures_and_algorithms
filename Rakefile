# frozen_string_literal: true

require 'rake'
require 'rake/clean'

# ========================================
# 编译器配置和构建标志
# ========================================

# 允许通过环境变量覆盖编译器，提高跨平台兼容性
CC      = ENV.fetch('CC', 'gcc')
AR      = ENV.fetch('AR', 'ar')

# 编译选项：启用所有警告、将警告视为错误、使用 C11 标准、
# 包含调试信息、关闭优化、添加头文件搜索路径
CFLAGS  = '-Wall -Wextra -Werror -std=c11 -g -O0 -Ilib'

# 链接选项：允许通过环境变量传入额外的链接标志
LDFLAGS = ENV.fetch('LDFLAGS', '')

# 静态库归档标志：r=替换已有文件、c=创建库、s=创建符号索引
ARFLAGS = 'rcs'

# ========================================
# 目录配置
# ========================================

LIB_DIR = 'lib'                          # 库源代码目录
BIN_DIR = 'bin'                          # 可执行文件源代码目录
OUT_DIR = 'out'                          # 构建输出目录

# 声明输出目录任务，Rake 会自动在需要时创建该目录
directory OUT_DIR

# ========================================
# 源文件和目标文件定义
# ========================================

# 库源文件列表（仅 lib/ 目录下一级 .c 文件）
LIB_SRCS = FileList["#{LIB_DIR}/*.c"]

# 可执行源文件列表（仅 bin/ 目录下一级 .c 文件）
BIN_SRCS = FileList["#{BIN_DIR}/*.c"]

# 库对象文件映射：lib/foo.c → out/foo.o
LIB_OBJS = LIB_SRCS.pathmap("#{OUT_DIR}/%n.o")

# 静态库目标文件
LIB_A = "#{OUT_DIR}/libdsa.a".freeze

# 可执行文件目标映射：bin/main.c → out/main（无扩展名）
BINS = BIN_SRCS.pathmap("#{OUT_DIR}/%n")

# ========================================
# 辅助函数
# ========================================

# 解析 gcc -MMD 生成的 .d 依赖文件，并动态为 Rake 任务添加头文件依赖。
# 实现增量构建能力：当头文件变更时，自动重新编译依赖它的目标文件。
def load_deps(dep_file, target)
  return unless File.exist?(dep_file)

  # 读取文件并合并续行（Makefile 风格的 \ 换行）
  content = File.read(dep_file).gsub("\\\n", ' ')

  # 匹配 "target: dep1 dep2 ..." 格式
  return unless content =~ /^([^:]+):\s*(.*)$/

  deps = Regexp.last_match(2).split(/\s+/).reject(&:empty?)

  # 仅添加磁盘上真实存在的文件（避免头文件被删除后 Rake 报错）
  existing_deps = deps.select { |d| File.exist?(d) }

  Rake::Task[target].enhance(existing_deps)
end

# ========================================
# 库构建规则：.c → .o 并打包静态库
# ========================================

# 为每个库源文件创建独立的对象文件构建任务
LIB_SRCS.each do |src|
  obj = src.pathmap("#{OUT_DIR}/%n.o")
  dep_file = obj.ext('d')

  # 文件任务：显式依赖输出目录和源文件
  # -MMD：生成 Makefile 风格的依赖文件（.d），记录源文件包含的头文件
  file obj => [OUT_DIR, src] do
    sh "#{CC} #{CFLAGS} -MMD -c #{src} -o #{obj}"
    # 编译后立即加载新生成的依赖文件，确保头文件变更能被追踪
    load_deps(dep_file, obj)
  end
end

# 静态库打包任务：依赖所有对象文件
file LIB_A => LIB_OBJS do |t|
  # 使用 ar 打包所有 .o 文件为静态库
  sh "#{AR} #{ARFLAGS} #{t.name} #{t.prerequisites.join(' ')}"
end

# ========================================
# 可执行文件构建规则：编译为 .o 再链接静态库
# ========================================

# 为每个 bin/*.c 创建中间对象文件任务
BIN_OBJS = BIN_SRCS.pathmap("#{OUT_DIR}/%n.bin.o")

BIN_SRCS.each do |src|
  obj = src.pathmap("#{OUT_DIR}/%n.bin.o")
  dep_file = "#{obj}.d"

  file obj => [OUT_DIR, src] do
    sh "#{CC} #{CFLAGS} -MMD -c #{src} -o #{obj}"
    # 编译后立即加载新生成的依赖文件
    load_deps(dep_file, obj)
  end

  # 为每个 bin/*.c 创建可执行文件构建任务
  exe = src.pathmap("#{OUT_DIR}/%n")
  obj = src.pathmap("#{OUT_DIR}/%n.bin.o")

  # 文件任务：依赖输出目录、对象文件和静态库
  file exe => [OUT_DIR, obj, LIB_A] do
    # 链接对象文件和静态库
    sh "#{CC} #{obj} #{LIB_A} #{LDFLAGS} -o #{exe}"
  end
end

# ========================================
# 加载自动生成的依赖文件
# ========================================

# 为库对象文件注入依赖（out/foo.o 对应 out/foo.d）
LIB_OBJS.each do |obj|
  load_deps(obj.ext('d'), obj)
end

# 为可执行文件对象文件注入依赖（out/main.bin.o 对应 out/main.bin.o.d）
BIN_OBJS.each do |obj|
  load_deps("#{obj}.d", obj)
end

# ========================================
# 顶层任务定义
# ========================================

# 构建所有目标
task all: [OUT_DIR, LIB_A] + BINS

# 默认任务
task default: :all

# ========================================
# 清理任务
# ========================================

# 将输出目录加入 CLEAN 任务（执行 rake clean 时删除 out/ 目录）
CLEAN << OUT_DIR
