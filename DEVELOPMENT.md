# Data Structures and Algorithms — AI Agent Guide

纯 C 实现的核心数据结构与算法教学代码库。Ruby **仅**用于 `Rakefile` 构建脚本。

`AGENTS.md` 指向本文件；本文件承载全部编码规范、构建命令和权限边界。

**代码即教材**——每行代码面向算法初学者，可读性优先于性能。

---

## 技术栈

| 用途 | 技术 | 约束 |
|------|------|------|
| 算法实现 | C | C11 (`-std=c11`) |
| 构建脚本 | Ruby | ≥ 3.3 (`gem install rake`) |
| 编译器 | Clang | (或兼容编译器) |
| 格式化 | `.clang-format` | LLVM 风格 |
| 测试框架 | 自定义 `TEST` 宏 | 定义于 `bin/test_*.c` |

---

## 目录结构

```
.
├── lib/                 # [C] 数据结构实现 (头文件 + 源文件)
├── bin/                 # [C] 测试与示例入口
├── out/                 # 构建输出 (gitignore)
├── Rakefile             # 构建脚本 (谨慎修改)
├── .clang-format        # C 格式化配置 (只读)
├── .clangd              # C 语言服务器配置 (只读)
└── .rubocop.yml         # Ruby 风格配置 (只读)
```

---

## 核心命令

```sh
# 构建
rake                          # 编译全部 lib/*.c + bin/*.c → out/

# C 测试
./out/test_<module>           # 运行各模块测试

# C 示例
./out/example_<name>          # 运行算法示例

# 清理
rake clean                    # 删除 out/
```

### 编译选项 (定义于 `Rakefile`)

| 选项 | 说明 |
|------|------|
| `-std=c11` | C11 标准 |
| `-Wall -Wextra -Werror` | 所有警告视为错误 |
| `-g -O0` | 调试信息，关闭优化 |
| `-MMD` | 自动生成头文件依赖 |
| `-Ilib` | 头文件搜索路径 |

构建流程：`lib/*.c` → `out/*.o` → `out/libdsa.a`；`bin/*.c` → `out/<name>.bin.o` → 链接静态库 → `out/<name>`。

---

## C 编码规范

### TEST 宏

每个 `bin/test_<module>.c` 必须定义并仅使用此宏编写测试：

```c
#define TEST(name, expr)                             \
    do                                               \
    {                                                \
        printf("  [测试] %-40s ", name);             \
        if (expr)                                    \
        {                                            \
            printf("✓ 通过\n");                      \
        }                                            \
        else                                         \
        {                                            \
            printf("✗ 失败 (第 %d 行)\n", __LINE__); \
            return 1;                                \
        }                                            \
    } while (0)
```

### 命名：位序 vs 下标

严格区分 **1-based 位序** (`ord`) 与 **0-based 数组下标** (`index`)。外部接口全部使用 `ord`，内部实现用 `index = ord - 1` 转换。

```c
// ✅ Good
bool SeqList_GetElem(SeqList *seq, int ord, int *elem);
int index = ord - 1;  // 位序 → 数组下标
return i + 1;         // 数组下标 → 位序

// ❌ Bad
bool SeqList_GetElem(SeqList *seq, int pos, int *elem); // pos 含义模糊
int idx = ord;  // 混淆位序与下标
```

### 函数命名

- 类型名：`PascalCase`，无 `_t` 后缀 (如 `Linked_List`)
- 函数名：`ModuleName_VerbNoun` (如 `Linked_List_InsertAtOrd`)
- 变量名：`camelCase` (如 `oldElem`)

### 风格速查

由 `.clang-format` (基于 LLVM) 定义：缩进 4 空格、Allman 大括号、`int *ptr` (PointerAlignment: Right)、行宽 80。

### 错误处理

- 返回 `bool` (成功/失败) 或 `int` (查找位序，失败返 0)。**禁用** `enum` 错误码
- 失败时调用 `printf("<FuncName> 错误描述\n")` 打印诊断，不静默失败

```c
// ✅ Good
if (ord < 1 || seq->Length < ord) {
    printf("<SeqList_GetElem> 位序 ord = %d 越界（表长 = %d）\n", ord, seq->Length);
    return false;
}

// ❌ Bad
if (pos < 0 || seq->length <= pos) return -1;  // 静默失败
```

### 内存管理

每个 `malloc`/`realloc` 必须配对 `free`。Destroy 后将所有指针置 `NULL`、所有计数字段归零。

```c
// ✅ Good
bool SeqList_Destroy(SeqList *seq) {
    if (!seq) { printf("<SeqList_Destroy> seq 为 NULL\n"); return false; }
    free(seq->Data);
    seq->Data = NULL;
    seq->Length = seq->MaxSize = 0;
    return true;
}

// ❌ Bad
void destroy(SeqList *s) { free(s->Data); } // 未置 NULL，未归零字段
```

---

## 注释规范

### 文件头部 (必须)

每个 `.h`/`.c` 文件必须包含块注释：数据结构名称+中文译名、核心术语定义、内部编码约定。

**仅允许 `//` 行注释，禁止 `/* */` 块注释。**

```c
// ==========================================================================
// 顺序表 (SeqList) —— 用动态数组实现的线性表
//
// 核心概念：
//   - 位序 (ord)：逻辑上第几个元素，从 1 开始计数，供外部调用者使用
//   - 下标 (index)：底层数组的下标，从 0 开始计数，仅在实现内部使用
//   - Length：当前元素个数（表长）
//   - MaxSize：已分配的数组最大容量
//
// 各函数在遇到参数无效等错误时，会打印清晰的错误消息并返回 false 或 0。
// ==========================================================================
```

### 函数分组 (推荐)

```c
// ---- 创建与销毁 ----
// ---- 状态查询 ----
// ---- 元素存取 ----
// ---- 查找 ----
// ---- 插入与删除（核心算法） ----
```

### 函数注释 (必须)

每个对外函数前说明：功能、参数约束、返回值含义、副作用。

### 核心算法注释 (必须)

涉及元素移动、指针操纵等关键算法时，必须包含：
- **算法步骤分解** (第 1 步 / 第 2 步 …)
- **ASCII 图示**展示数据变化
- **时间复杂度**

### 行内注释

解释**为什么**而非做什么。关键转换处 (`ord ↔ index`) 必须注释。

```c
// ✅ Good
int index = ord - 1;  // 位序 → 数组下标

// ❌ Bad
// 把 ord 减 1 赋值给 index
int index = ord - 1;
```

---

## 测试策略

添加新模块流程：1) 创建 `lib/<module>.h` + `lib/<module>.c`  2) 创建 `bin/test_<module>.c` (含 `TEST` 宏)  3) `rake` 编译  4) `./out/test_<module>` 验证。

必测场景：**基本 CRUD**、**边界检查** (ord=0、越界、空表、单元素)、**NULL 指针** (每个指针参数)、**扩容** (超初始容量 16)、**综合场景** (头插/尾追/中间/混合)。

---

## 常见陷阱

| # | ❌ 错误 | ✅ 正确 |
|---|---------|--------|
| 1 | `pos`/`idx` 混淆位序与下标 | 外部 `ord`，内部 `index` |
| 2 | 静默返回 -1，无诊断 | `printf("<FuncName> 错误描述\n")` |
| 3 | 定义 `enum` 错误码 | 返回 `bool`，失败时已打印诊断 |
| 4 | `free` 后不置 NULL / 不归零字段 | Destroy 后所有字段归零 |
| 5 | 为"优化"牺牲可读性 | 教学项目：清晰 > 性能 |
| 6 | 使用非标准 C 扩展 | 仅用 C11 标准特性 |
| 7 | 删除或缩短教学性注释 | 注释即教材，必须保留 |

---

## 已实现模块

| 模块 | 文件 | 说明 |
|------|------|------|
| 顺序表 | `lib/seq_list.h/.c` | 动态数组线性表，支持 CRUD + 自动扩容 |
| 双向链表 | `lib/linked.h/.c` | 支持按位序 CRUD + 栈 (LIFO) + 队列 (FIFO) |
| 并查集 | `lib/uf_set.h/.c` | 双亲表示法 Union-Find |
| 表达式求值 | `bin/example_stack_for_expr_conversion_and_evaluation.c` | 调度场算法 + 后缀求值 |

---

## 权限边界

### ✅ Always
- 修改 C 代码后运行 `rake` 确保编译通过，再运行 `./out/test_<module>` 验证
- 保持完整的块注释、函数注释、行内注释
- 遵循 `ord`/`index` 命名约定，`free` 后置 NULL 并归零字段

### ⚠️ Ask First
- 重构跨模块公共接口
- 修改 `Rakefile` 或构建流程
- 添加外部依赖 (C 库)
- 修改 `.clang-format`、`.clangd`、`.rubocop.yml`
- 修改编译选项 (CFLAGS / LDFLAGS)
- 新增目录 / 子目录

### 🚫 Never
- 使用非标准 C 扩展
- 使用 `enum` 错误码、拼音 / 拼音缩写命名
- 使用 `/* */` 块注释
- 提交二进制文件到 Git
- 删除或缩短教学性注释
- 关闭 `-Werror`
- 在核心算法中引入第三方库
