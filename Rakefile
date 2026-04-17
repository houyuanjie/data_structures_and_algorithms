# frozen_string_literal: true

require 'rake'
require 'rake/clean'

# ========================================
# 编译器配置和构建标志
# ========================================

CC      = 'gcc'                          # C 编译器
CFLAGS  = '-Wall -Wextra -g -O0 -Ilib'   # 编译选项
LDFLAGS = ''                             # 链接选项
AR      = 'ar'                           # 静态库归档工具
ARFLAGS = 'rcs'                          # 归档标志：r=替换、c=创建、s=创建索引

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
LIB_A = "#{OUT_DIR}/libdsa.a"

# 可执行文件目标映射：bin/main.c → out/main（无扩展名）
BINS = BIN_SRCS.pathmap("#{OUT_DIR}/%n")

# ========================================
# 库构建规则：.c → .o 并打包静态库
# ========================================

# 为每个库源文件创建独立的对象文件构建任务
LIB_SRCS.each do |src|
  obj = src.pathmap("#{OUT_DIR}/%n.o")

  # 文件任务：显式依赖输出目录和源文件
  # -MMD：生成 Makefile 风格的依赖文件（.d）
  # -MP：为头文件添加“伪目标”，防止头文件被删除后 make 报错
  file obj => [OUT_DIR, src] do
    sh "#{CC} #{CFLAGS} -MMD -MP -c #{src} -o #{obj}"
  end
end

# 静态库打包任务：依赖所有对象文件
file LIB_A => LIB_OBJS do |t|
  # 使用 ar 打包所有 .o 文件为静态库
  sh "#{AR} #{ARFLAGS} #{t.name} #{t.prerequisites.join(' ')}"
end

# ========================================
# 可执行文件构建规则：编译 + 链接静态库
# ========================================

# 为每个 bin/*.c 创建可执行文件构建任务
BIN_SRCS.each do |src|
  exe = src.pathmap("#{OUT_DIR}/%n")
  dep = "#{exe}.bin.d"   # 使用 .bin.d 后缀，防止与库对象的 .d 文件同名冲突

  # 文件任务：显式依赖输出目录、源文件和静态库
  file exe => [OUT_DIR, src, LIB_A] do
    # 编译并链接（-MMD -MP 生成依赖文件，-MF 指定依赖文件名）
    # 注意：源文件放在静态库之前，符合链接器符号解析顺序
    sh "#{CC} #{CFLAGS} -MMD -MP -MF #{dep} #{src} #{LIB_A} #{LDFLAGS} -o #{exe}"
  end
end

# ========================================
# 加载自动生成的依赖文件
# ========================================

# 辅助函数：解析 gcc -MMD 生成的 .d 文件，并动态为 Rake 任务添加头文件依赖
# 实现类似 Make 的增量构建能力
def load_deps(dep_file, target)
  return unless File.exist?(dep_file)

  # 读取文件并合并续行（Makefile 风格的 \ 换行）
  content = File.read(dep_file).gsub(/\\\n/, ' ')

  # 匹配 "target: dep1 dep2 ..." 格式
  return unless content =~ /^([^:]+):\s*(.*)$/

  deps = Regexp.last_match(2).split(/\s+/).reject(&:empty?)

  # 仅添加磁盘上真实存在的文件（避免头文件被删除后 Rake 报错）
  existing_deps = deps.select { |d| File.exist?(d) }

  Rake::Task[target].enhance(existing_deps)
end

# 为库对象文件注入依赖（out/foo.o 对应 out/foo.d）
LIB_OBJS.each do |obj|
  load_deps(obj.ext('d'), obj)
end

# 为可执行文件注入依赖（out/main 对应 out/main.bin.d）
BINS.each do |exe|
  load_deps("#{exe}.bin.d", exe)
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
