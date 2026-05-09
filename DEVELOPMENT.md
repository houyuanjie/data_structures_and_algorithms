# Data Structures and Algorithms — AI Agent Guide

《数据结构与算法》教学代码库。C 实现核心数据结构，Ruby 驱动测试与算法演示。
**代码即教材**——每行代码都面向算法初学者，可读性优先于性能。

### 关于本文件

本文件 `DEVELOPMENT.md` 是项目的**"机器级 README"**，是 AI 编程代理参与本项目时
的首要参考文档。`AGENTS.md` 是 GitHub Copilot 等 AI 工具的标准入口文件，
其内容为 `@DEVELOPMENT.md`，将 AI 代理重定向至此文件。

- **`AGENTS.md`**：AI 代理入口点，仅包含一行 `@DEVELOPMENT.md` 引用
- **`DEVELOPMENT.md`**（本文件）：承载全部编码规范、构建命令、权限边界

修改开发规范时，只需编辑 `DEVELOPMENT.md`。不要修改 `AGENTS.md` 中的引用格式。

---

## 技术栈

| 层 | 技术 | 版本/标准 |
|---|------|----------|
| 算法核心 | C | C11 (`-std=c11`) |
| 构建/测试/演示 | Ruby | ≥ 3.3 (内置 Prism) |
| 构建系统 | Rake | `gem install rake` |
| C 编译器 | GCC | (或兼容编译器) |
| C 测试框架 | 自定义 `TEST` 宏 | 定义于 `bin/test_*.c` |
| Ruby 测试框架 | Minitest | RubyGems 内置 |

## 目录结构

```
.
├── lib/                  # [C] 数据结构实现 —— 可写
│   ├── seq_list.h/.c     #   顺序表（动态数组）
│   ├── uf_set.h/.c       #   并查集（双亲表示法）
│   └── hello_world.h/.c  #   构建验证占位
├── bin/                  # [C] 测试程序入口 —— 可写
│   ├── test_seq_list.c   #   顺序表测试
│   ├── test_uf_set.c     #   并查集测试
│   └── hello_world.c     #   构建验证
├── Snippets/             # [Ruby] 算法演示片段 —— 可写
│   └── stack/
│       ├── to_postfix.rb #   调度场算法：中缀→后缀
│       └── eval_postfix.rb # 栈求值：后缀表达式计算
├── out/                  # 构建输出 —— 只读（gitignore）
├── Rakefile              # 构建系统 —— 谨慎修改
└── .clangd .rubocop.yml  # 工具配置 —— 只读
```

---

## 核心命令

```sh
# ---- 构建 ----
rake                          # 编译全部，生成 out/libdsa.a + 可执行文件

# ---- C 测试 ----
./out/test_seq_list            # 顺序表测试
./out/test_uf_set              # 并查集测试

# ---- Ruby 算法演示（含测试） ----
ruby Snippets/stack/to_postfix.rb    # 中缀→后缀（含 19 个测试）
ruby Snippets/stack/eval_postfix.rb  # 后缀求值（含 22 个测试）

# ---- 清理 ----
rake clean                     # 删除 out/
```

### 编译选项（定义于 `Rakefile`）

| 选项 | 说明 |
|------|------|
| `-std=c11` | C11 标准 |
| `-Wall -Wextra -Werror` | 启用所有常见警告并视为错误 |
| `-g -O0` | 调试信息、关闭优化 |
| `-MMD` | 自动生成头文件依赖（增量构建） |
| `-Ilib` | 头文件搜索路径 |

Rake 构建流程：`lib/*.c` → `out/*.o` → `out/libdsa.a`；`bin/*.c` + 链接静态库 → `out/<name>` 可执行文件。

---

## C 编码规范

### TEST 宏（测试文件模板）

每个 `bin/test_<module>.c` 必须使用以下宏：

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

区分 1-based 位序与 0-based 数组下标，从名称即可判断。

✅ Good：
```c
bool SeqList_GetElem(SeqList *seq, int ord, int *elem);  // ord = 位序，供外部调用者使用
int index = ord - 1;  // 位序 → 数组下标（内部实现用）
```

❌ Bad：
```c
bool SeqList_GetElem(SeqList *seq, int pos, int *elem);  // pos 含义模糊：是位序还是下标？
int idx = ord;  // 混淆了位序与下标
```

### 风格速查

缩进 4 空格，Allman 大括号，`int *ptr`（星靠变量），`PascalCase` 类型无 `_t` 后缀，`ModuleName_VerbNoun` 函数名，行宽 ≤ 100。

### 错误处理与内存管理

返回 `bool`（true=成功）或 `int`（失败返 0），禁用 `enum` 错误码。失败时 `printf` 带函数名诊断。每个 `malloc`/`realloc` 配对 `free`，Destroy 后指针置 NULL 并归零所有字段。

✅ Good：
```c
if (ord < 1 || seq->Length < ord) {
    printf("<SeqList_GetElem> 位序 ord = %d 越界（表长 = %d）\n", ord, seq->Length);
    return false;
}
bool SeqList_Destroy(SeqList *seq) {
    if (!seq) { printf("<SeqList_Destroy> seq 为 NULL\n"); return false; }
    free(seq->Data);
    seq->Data = NULL;
    seq->Length = seq->MaxSize = 0;
    return true;
}
```

❌ Bad：
```c
if (pos < 0 || seq->length <= pos) return -1;  // 静默失败，无诊断
void destroy(SeqList *s) { free(s->Data); }    // 未置 NULL，未重置字段
```

---

## Ruby 编码规范

首行 `# frozen_string_literal: true`，缩进 2 空格。[YARD](https://yardoc.org/) `@param`/`@return` 类型标注。[Minitest](https://docs.seattlerb.org/minitest/) 测试，类名 `Test<Subject>`，方法名 `test_<scenario>`。脚本独立可执行 `ruby <file>.rb`。

✅ Good：
```ruby
# @param expression [String] 中缀表达式，操作数间以空格分隔
# @return [String] 后缀表达式
def to_postfix(expression)
```

❌ Bad：
```ruby
def to_postfix(exp)  # 无类型标注，参数名模糊
```

---

## 教学注释规范

### 文件头部（必须）

每个 `.h`/`.c` 文件头部必须包含块注释：数据结构名称+中文译名、核心术语定义、内部编码约定。

✅ Good：
```c
/* ==========================================================================
 * 顺序表 (SeqList) —— 用动态数组实现的线性表
 *
 * 核心概念：
 *   - 位序 (ord)：逻辑上第几个元素，从 1 开始计数，供外部调用者使用
 *   - 下标 (index)：底层数组的下标，从 0 开始计数，仅在实现内部使用
 *   - Length：当前元素个数（表长）
 *   - MaxSize：已分配的数组最大容量
 *
 * 各函数在遇到参数无效等错误时，会打印清晰的错误消息并返回 false 或 0，
 * 调用者无需再区分不同的错误码，专注于返回值所代表的成功/失败语义。
 * ========================================================================== */
```

### 函数分组（推荐）

用 `/* ---- 组名 ---- */` 将相关函数分为逻辑区块：

```c
/* ---- 创建与销毁 ---- */
/* ---- 状态查询 ---- */
/* ---- 元素存取 ---- */
/* ---- 查找 ---- */
/* ---- 插入与删除（核心算法） ---- */
```

### 函数注释（必须）

每个对外函数前说明：功能、参数含义和约束、返回值含义、副作用。

### 核心算法注释（强烈推荐）

涉及元素移动、指针追溯等关键算法时，必须包含：
- **算法步骤分解**（第 1 步 / 第 2 步 …）
- **ASCII 图示**展示数据变化
- **时间复杂度**

### 行内注释

解释**为什么**而非做什么（代码已说明做了什么）。关键转换处（ord ↔ index）必须注释。

✅ Good：
```c
int index = ord - 1;  // 位序 → 数组下标
return i + 1;         // 数组下标 → 位序
```

❌ Bad：
```c
// 把 ord 减 1 赋值给 index
int index = ord - 1;
// 把 i 加 1 后返回
return i + 1;
```

---

## 测试策略

添加新 C 数据结构：1) 创建 `lib/<module>.h` 和 `lib/<module>.c`  2) 创建 `bin/test_<module>.c`（含 `TEST` 宏） 3) `rake` 编译  4) `./out/test_<module>` 验证。

必测场景：基本操作（CRUD）、边界检查（ord=0/越界/空表/单元素）、NULL 指针（每个指针参数）、扩容（超初始容量 16）、综合场景（头插/尾追/中间/混合）。

---

## 常见陷阱

| # | ❌ 错误 | ✅ 正确 |
|---|---------|--------|
| 1 | `pos`/`idx` 混淆位序与下标 | 外部 `ord`，内部 `index` |
| 2 | 静默返回 -1，无诊断 | `printf("<FuncName> 错误描述\n")` |
| 3 | 定义 `enum` 错误码 | 返回 `bool`，失败时已打印诊断 |
| 4 | `free` 后不置 NULL/不重置字段 | Destroy 后所有字段归零 |
| 5 | C 中写 Ruby 语法，反之亦然 | 严格遵守双栈边界 |
| 6 | 为"优化"牺牲可读性 | 教学项目：清晰 > 性能 |
| 7 | 使用非标准 C 扩展 | 仅用 C11 标准特性 |
| 8 | 删除教学性注释 | 注释即教材，必须保留 |

---

## 已实现的数据结构与算法

### C 数据结构

| 模块 | 文件 | 说明 |
|------|------|------|
| 顺序表 | `lib/seq_list.h/.c` | 动态数组线性表，支持 CRUD + 自动扩容 |
| 并查集 | `lib/uf_set.h/.c` | 双亲表示法 Union-Find，支持 Find/Union |

### Ruby 算法片段

| 片段 | 文件 | 说明 |
|------|------|------|
| 中缀→后缀 | `Snippets/stack/to_postfix.rb` | 调度场算法，含 19 个测试 |
| 后缀求值 | `Snippets/stack/eval_postfix.rb` | 操作数栈求值，含 22 个测试 |

---

## 权限边界

### ✅ Always
- 修改 C 代码后运行 `rake` 确保编译通过，然后运行 `./out/test_<module>`
- 保持完整的块注释、函数注释、行内注释
- 遵循 `ord`/`index` 命名约定，`free` 后置 NULL 并重置字段

### ⚠️ Ask First
- 重构跨模块公共接口、修改 `Rakefile`/构建流程
- 添加外部依赖（C 库或 Ruby gem）
- 修改 `.clangd`/`.rubocop.yml`、编译选项（CFLAGS/LDFLAGS）
- 新增章节子目录

### 🚫 Never
- 使用非标准 C 扩展、`enum` 错误码、拼音/拼音缩写命名
- 提交二进制文件、删除/缩短教学性注释
- 在 C 中引入 Ruby 逻辑（反之亦然）、关闭 `-Werror`
- 在核心算法中引入第三方库
