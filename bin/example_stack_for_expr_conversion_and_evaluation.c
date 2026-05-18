#define _GNU_SOURCE

#include "linked.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// ==========================================================================
// 表达式转换与求值 —— 用栈实现中缀→后缀转换（调度场算法）和后缀表达式求值
//
// 核心概念：
//   - 调度场算法 (Shunting Yard Algorithm)：E.W. Dijkstra 提出的将中缀表达式
//     转换为后缀表达式（逆波兰表示法，RPN）的算法。借助运算符栈，根据运算符
//     优先级和括号，重新排列运算符和操作数的顺序。
//   - 后缀表达式求值：从左到右扫描后缀表达式 token，遇到操作数则压栈，
//     遇到运算符则弹出栈顶两个操作数计算后压回栈。扫描结束，栈中剩下的
//     唯一值即为结果。无需括号，无需优先级表。
//   - 栈 (Stack)：使用双向链表 (Linked_List) 通过 Push/Pop/Peek 操作实现
//     LIFO（后进先出）行为。
//   - Arena 分配器：用于构建后缀表达式字符串，通过 string builder 逐 token
//     追加，最后统一释放。避免手动 free，同时演示 Arena 的 SB（字符串构建器）宏。
//
// 算法：
//   1. to_postfix：调度场算法，O(n) 时间 / O(n) 空间
//   2. eval_postfix：后缀求值，O(n) 时间 / O(n) 空间
//
// 示例：
//   中缀: ((15/(7-(1+1)))*3)-(2+(1+1))
//   后缀: 15 7 1 1 + - / 3 * 2 1 1 + + -
//   结果: 5
// ==========================================================================

// --------------------------------------------------------------------------
// 运算符优先级表
//
// 数值越大优先级越高；相同优先级按从左到右结合（在弹栈比较时用 >= 实现）。
// --------------------------------------------------------------------------

static int precedence(char op)
{
    switch (op)
    {
    case '*':
    case '/':
        return 2;
    case '+':
    case '-':
        return 1;
    default:
        return 0;
    }
}

// --------------------------------------------------------------------------
// Arena 字符串构建器 —— 用于逐 token 拼接后缀表达式
//
// 声明 char* 字符串构建器结构体，配合 arena_sb_append_buf 等宏使用。
// items  = 字符串首地址；count = 已填入的字符数（不含 '\0'）；
// capacity = 当前容量（数组可容纳的字符数）。
// 初始时全部字段为 0，Arena 宏会在首次追加时自动分配内存。
// --------------------------------------------------------------------------

typedef struct
{
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;

// ==========================================================================
// to_postfix —— 调度场算法（Shunting Yard Algorithm）
//
// 将中缀表达式转换为后缀表达式（逆波兰表示法，RPN）。
// 使用双向链表 (Linked_List) 作为运算符栈，
// 使用 Arena string builder 构建输出字符串。
//
// 算法步骤：
//   第 1 步：遍历输入字符串，识别 token（操作数 / 运算符 / 括号）
//   第 2 步：按调度场算法处理每个 token：
//     (a) 操作数 → 直接追加到 string builder
//     (b) '('    → 直接入栈
//     (c) ')'    → 弹栈输出直到遇到 '('，丢弃括号对
//     (d) 运算符 → 弹栈中所有优先级 >= 当前运算符的运算符并输出，
//                  然后将当前运算符入栈
//   第 3 步：输入结束后，将栈中剩余运算符全部弹栈输出
//
// @param arena  用于分配输出字符串的 Arena 指针
// @param expr   中缀算术表达式字符串，支持多位数操作数
// @return       Arena 分配的后缀表达式 C 字符串（'\0' 结尾），
//               失败返回 NULL。调用者无需单独 free，由 Arena 统一释放。
//
// 时间复杂度：O(n) —— 每个 token 均入栈、出栈各至多一次
// 空间复杂度：O(n) —— 运算符栈和输出字符串的大小与输入规模成线性关系
// ==========================================================================

char *to_postfix(Arena *arena, const char *expr)
{
    if (arena == NULL)
    {
        printf("<to_postfix> arena 为 NULL\n");
        return NULL;
    }
    if (expr == NULL)
    {
        printf("<to_postfix> expr 为 NULL\n");
        return NULL;
    }

    // ====================================================================
    // 第 0 步：初始化 —— 准备好运算符栈和后缀字符串构建器
    // ====================================================================

    // op_stack: 栈顶 = 表头 (Head)，入栈出栈均为 O(1)
    Linked_List op_stack;
    Linked_List_Initialize(&op_stack);

    // sb: arena 托管的后缀字符串构建器
    // {0} 将所有字段初始化为 0/NULL，首次 append 时 arena 自动分配内存
    String_Builder sb = {0};

    // first_token: 标记是否是第一个输出 token
    // true 时直接输出数字，false 时先输出一个空格分隔符再输出数字
    bool first_token = true;

    // result: 返回值，初始为 NULL
    // 成功时在 cleanup 前设为 sb.items，失败时保持 NULL
    char *result = NULL;

    // ====================================================================
    // 第 1 步：逐字符扫描中缀表达式，按字符类型分支处理
    //
    // 分支结构：
    //   (A) 空白字符 → 跳过
    //   (B) 数字     → 读取完整多位数，追加到 sb
    //   (C) '('      → 直接入栈
    //   (D) ')'      → 弹栈输出直到遇到 '('
    //   (E) 运算符   → 弹栈中优先级 >= 自己的运算符，然后自己入栈
    // ====================================================================

    int index = 0; // index: 当前扫描位置（0-based 数组下标）
    while (expr[index] != '\0')
    {
        char ch = expr[index]; // 当前字符

        // ================================================================
        // 分支 (A)：空白字符 → 跳过
        // isspace 是 <ctype.h> 标准函数，检查字符是否为空格/制表/换行等。
        // 将 char 转为 unsigned char 是为了避免负值传入导致的未定义行为。
        // ================================================================
        if (isspace((unsigned char)ch))
        {
            index++;
            continue; // 跳过空白，处理下一个字符
        }

        // ================================================================
        // 分支 (B)：数字 → 读取完整操作数，追加到输出
        //
        // 数字可能是多位数（如 15、123），需要一个内层循环将其完整读出。
        // isdigit 是 <ctype.h> 标准函数，判断字符是否为 '0'~'9'。
        // 读取结束时 index 已指向操作数之后第一个非数字字符，
        // 外层循环的下次迭代将直接处理它。
        // ================================================================
        if (isdigit((unsigned char)ch))
        {
            int num = 0; // 累加解析出的数值
            while (isdigit((unsigned char)expr[index]))
            {
                num = num * 10 + (expr[index] - '0');
                index++;
            }

            // 将操作数的字符串表示写入临时缓冲区
            char buf[32];
            int len;
            if (first_token)
            {
                // 第一个 token 前不加空格
                // snprintf: 将格式化内容写入 buf，返回实际写入的字符数(不含 '\0')
                len = snprintf(buf, sizeof(buf), "%d", num);
                first_token = false;
            }
            else
            {
                // 后续 token 前添加一个空格分隔符
                len = snprintf(buf, sizeof(buf), " %d", num);
            }

            // arena_sb_append_buf: 将 buf 中 len 个字符追加到 sb 字符串构建器；
            // 容量不足时 arena 自动扩容（新容量 = max(256, 当前容量×2)）
            arena_sb_append_buf(arena, &sb, buf, len);

            continue; // 已处理完操作数，继续扫描下一个字符
        }

        // 执行到这里说明 ch 不是空白也不是数字
        // 先前进 index 越过当前字符。
        char token = ch;
        index++;

        // 非法字符检查：只允许 '(' ')' '+' '-' '*' '/' 五种运算符/括号
        if (token != '(' && token != ')' && token != '+' && token != '-' &&
            token != '*' && token != '/')
        {
            int ord = index;  // 下标 → 位序（index 已在上方递增，此时 index 恰好等于 ord）
            printf("<to_postfix> 非法字符 '%c'（位序 %d）\n", token, ord);
            goto cleanup;
        }

        // ================================================================
        // 分支 (C)：'(' → 无条件入栈
        //
        // '(' 是右括号 ')' 弹栈的边界标记 —— 弹栈时一遇到 '(' 就停止。
        // ================================================================
        if (token == '(')
        {
            Linked_Stack_Push(&op_stack, (int)'(');
            continue;
        }

        // ================================================================
        // 分支 (D)：')' → 弹栈输出，直到遇到 '(' 并丢弃该 '('
        //
        // 括号内的所有运算符按调度场规则已在栈中排好序，
        // 此处依次弹出输出即可还原括号内的运算顺序。
        //
        // 示例（括号内的表达式 `7-(1+1)`）：
        //   输入 '(' 后栈:  [  (  -  (  ]
        //   输入 ')' 后弹栈: 输出 '+' 、丢弃栈中 '('  → 栈:  [  (  -  ]
        //
        //   [栈]  (  op1  op2     ← 栈顶在表头
        //               ↑ 逐个弹出 op2、op1 输出，丢弃 '('
        // ================================================================
        if (token == ')')
        {
            // 在栈中寻找匹配的 '('
            // found_paren: 是否在栈中找到 '('
            bool found_paren = false;
            int top; // 栈顶运算符（ASCII 字符码）
            while (!Linked_List_IsEmpty(&op_stack))
            {
                Linked_Stack_Peek(&op_stack, &top);
                if (top == '(')
                {
                    found_paren = true;
                    break; // 遇到 '('，停止弹栈输出
                }
                // 弹出栈顶运算符并追加到输出
                Linked_Stack_Pop(&op_stack, &top);
                char buf[4];
                int len = snprintf(buf, sizeof(buf), " %c", (char)top);
                arena_sb_append_buf(arena, &sb, buf, len);
            }
            // 若未找到 '('，说明右括号没有匹配的左括号
            if (!found_paren)
            {
                printf("<to_postfix> 右括号 ')' 没有匹配的左括号\n");
                goto cleanup;
            }
            // 弹出并丢弃栈顶的 '('
            Linked_Stack_Pop(&op_stack, &top);
            continue;
        }

        // ================================================================
        // 分支 (E)：二元运算符 (+ - * /) → 弹栈 + 入栈
        //
        // 调度场核心规则：确保栈中运算符按优先级从低到高（栈底→栈顶）排列。
        //
        // 弹栈条件（满足任一即停止弹出）：
        //   ① 栈空            → 没有运算符可以比较
        //   ② 栈顶为 '('      → 括号内的运算符不能跨括号弹出
        //   ③ 栈顶优先级 < 当前优先级 → 栈顶被"保护"（低优先级的不能在高优先级之上弹出）
        //
        // 实现为：
        //   while (栈非空 && 栈顶 != '(' && 栈顶优先级 >= 当前优先级)
        //       弹栈输出
        //   然后当前运算符入栈
        //
        // 优先检查 IsEmpty 而非直接 Peek，避免空栈时 Peek 打印不必要的诊断消息。
        // ================================================================

        // 当前 token 必然是运算符，计算其优先级
        int cur_prec = precedence(token);

        // 弹栈：将优先级不低于当前运算符的栈内运算符全部弹出输出
        int top; // 栈顶运算符（ASCII 字符码）
        while (!Linked_List_IsEmpty(&op_stack))
        {
            Linked_Stack_Peek(&op_stack, &top);

            // 条件②：栈顶为 '(' 则停止
            if (top == '(')
            {
                break;
            }

            // 条件③：栈顶优先级 < 当前优先级则停止
            if (precedence((char)top) < cur_prec)
            {
                break;
            }

            // 栈顶优先级 >= 当前优先级：弹出并追加到输出
            Linked_Stack_Pop(&op_stack, &top);
            char buf[4];
            int len = snprintf(buf, sizeof(buf), " %c", (char)top);
            arena_sb_append_buf(arena, &sb, buf, len);
        }

        // 当前运算符入栈
        Linked_Stack_Push(&op_stack, (int)token);
    }

    // ====================================================================
    // 第 2 步：输入扫描完毕，检查空表达式和栈中剩余 token
    //
    // 输入中已经没有未处理 token，此时检查：
    //   (a) 空表达式：从未输出过操作数 → 报错
    //   (b) 栈中剩余 '(' → 未匹配的左括号，报错
    //   (c) 栈中剩余运算符 → 正常弹出输出
    // ====================================================================

    // 若第一个 token 从未被输出，说明表达式为空
    if (first_token)
    {
        printf("<to_postfix> 表达式为空\n");
        goto cleanup;
    }

    int top; // 栈顶运算符
    while (!Linked_List_IsEmpty(&op_stack))
    {
        Linked_Stack_Pop(&op_stack, &top);
        // 剩余栈中若有 '('，说明存在未匹配的左括号
        if (top == '(')
        {
            printf("<to_postfix> 左括号 '(' 没有匹配的右括号\n");
            goto cleanup;
        }
        char buf[4];
        int len = snprintf(buf, sizeof(buf), " %c", (char)top);
        arena_sb_append_buf(arena, &sb, buf, len);
    }

    // ====================================================================
    // 第 3 步：收尾
    //
    // arena_sb_append_null: 在 sb 末尾追加一个 '\0'（空字符），
    // 使 sb.items 成为合法的 C 风格 '\0' 结尾字符串，可供 printf 等使用。
    // ====================================================================

    arena_sb_append_null(arena, &sb);

    result = sb.items;

cleanup:
    // 释放运算符栈（栈结点由 malloc 分配，独立于 Arena 管理）
    Linked_List_Destroy(&op_stack);

    // 返回 arena 托管的后缀表达式字符串
    // 成功时 result = sb.items，失败时 result = NULL
    // 调用者无需单独 free，由 Arena 统一释放
    return result;
}

// ==========================================================================
// eval_postfix —— 后缀表达式求值
//
// 对后缀表达式（逆波兰表示法）求值，将结果写入 *result。
// 使用双向链表 (Linked_List) 作为操作数栈。
//
// 算法步骤：
//   第 1 步：从左到右扫描后缀表达式 token（空格分隔）
//   第 2 步：对每个 token：
//     (a) 操作数 → 转换为整数后压栈
//     (b) 运算符 → 弹出栈顶两个操作数（先弹右后弹左），
//                  计算后压回栈
//   第 3 步：扫描结束，栈中应有且仅有 1 个值，即为最终结果
//
// 注意：运算符弹出操作数的顺序是「先右后左」（先弹的是右操作数），
//       这对加法和乘法无影响，但对减法和除法至关重要。
//
// 可检测的错误：
//   - 表达式为空
//   - 非法字符（非数字、非运算符、非空格）
//   - 栈下溢（运算符缺少操作数）
//   - 除数为零
//   - 操作数过多（扫描结束后栈中仍有多余值）
//
// @param postfix   以空格分隔的后缀表达式 C 字符串（'\0' 结尾）
// @param result    输出参数，成功时写入计算结果
// @return          true 表示求值成功；false 表示表达式非法
//
// 时间复杂度：O(n) —— 每个 token 处理一次
// 空间复杂度：O(n) —— 操作数栈的大小
// ==========================================================================

bool eval_postfix(const char *postfix, int *result)
{
    if (postfix == NULL)
    {
        printf("<eval_postfix> postfix 为 NULL\n");
        return false;
    }
    if (result == NULL)
    {
        printf("<eval_postfix> result 为 NULL\n");
        return false;
    }

    // ====================================================================
    // 第 0 步：初始化 —— 操作数栈用于暂存中间计算结果
    // ====================================================================

    Linked_List val_stack;
    Linked_List_Initialize(&val_stack);

    // ret: 函数返回值，初始为 false（失败），成功时设为 true
    bool ret = false;

    // ====================================================================
    // 第 1 步：逐 token 扫描后缀表达式
    //
    // 后缀表达式中 token 以空格分隔，每个 token 要么是操作数（十进制数字），
    // 要么是单字符运算符（+ - * /）。
    //
    // 分支结构：
    //   (A) 空格           → 跳过
    //   (B) 运算符 (+-*/)  → 弹出两个操作数，计算后压回栈
    //   (C) 操作数（数字） → 转换为整数后压栈
    //
    // 判断 token 类型的依据：
    //   若当前字符是运算符且后跟空格或位于结尾 → 分支 (B)
    //   否则 → 分支 (C)
    // ====================================================================

    const char *scan = postfix; // scan: 扫描指针，指向当前待处理的字符
    while (*scan != '\0')
    {
        // 跳过前导空格
        while (*scan == ' ')
        {
            scan++;
        }
        if (*scan == '\0')
        {
            break; // 字符串末尾全是空格
        }

        // ================================================================
        // 分支 (B)：遇到运算符 (+ - * /)
        //
        // 运算符后紧跟空格或字符串结尾时才确认为运算符 token（区别于
        // 负号等场景，虽然本例不支持负号）。
        //
        // 对于减法和除法，操作数弹出的顺序至关重要：
        //   - 先 Pop 得到的是右操作数 right  （后入栈，靠近栈顶）
        //   - 再 Pop 得到的是左操作数 left   （先入栈，在栈中更深）
        //   - 计算 left op right
        // ================================================================
        char ch = *scan;
        bool is_operator = (ch == '+' || ch == '-' || ch == '*' || ch == '/');
        if (is_operator && (*(scan + 1) == ' ' || *(scan + 1) == '\0'))
        {
            char op = ch;
            scan++; // 前进到下一个 token

            // 先弹右操作数（后入栈），再弹左操作数（先入栈）
            int right, left;
            // Linked_Stack_Pop 在栈空时返回 false
            if (!Linked_Stack_Pop(&val_stack, &right))
            {
                printf("<eval_postfix> 运算符 '%c' 缺少操作数\n", op);
                goto cleanup;
            }
            if (!Linked_Stack_Pop(&val_stack, &left))
            {
                printf("<eval_postfix> 运算符 '%c' 缺少操作数\n", op);
                goto cleanup;
            }

            int value;
            switch (op)
            {
            case '+':
                value = left + right;
                break;
            case '-':
                value = left - right;
                break;
            case '*':
                value = left * right;
                break;
            case '/':
                if (right == 0)
                {
                    printf("<eval_postfix> 除数为零\n");
                    goto cleanup;
                }
                value = left / right;
                break;
            default:
                value = 0;
                break;
            }

            // 将计算结果压回栈，供后续运算使用
            Linked_Stack_Push(&val_stack, value);

            continue; // 继续处理下一个 token
        }

        // ================================================================
        // 分支 (C)：遇到操作数 → 转换为整数后压栈
        //
        // strtol 是 <stdlib.h> 标准函数：
        //   strtol(待解析字符串, &结束位置指针, 进制基数)
        //   从 p 开始解析十进制整数，解析结束时 *endptr 指向
        //   第一个非数字字符（此处是空格或 '\0'）。
        //   然后将 scan 跳到 endptr，准备处理下一个 token。
        // ================================================================
        if (!isdigit((unsigned char)ch))
        {
            printf("<eval_postfix> 非法字符 '%c'\n", ch);
            goto cleanup;
        }

        char *endptr;
        int num = (int)strtol(scan, &endptr, 10);
        Linked_Stack_Push(&val_stack, num);
        scan = endptr; // 跳到下一个 token 的开始位置
    }

    // ====================================================================
    // 第 2 步：扫描结束，栈中应有且仅有 1 个值，即为最终结果
    // ====================================================================

    int value;
    if (!Linked_Stack_Pop(&val_stack, &value))
    {
        printf("<eval_postfix> 表达式为空\n");
        goto cleanup;
    }
    // 栈中不应有多余值（若仍有元素，说明操作数过多）
    if (!Linked_List_IsEmpty(&val_stack))
    {
        printf("<eval_postfix> 操作数过多\n");
        goto cleanup;
    }

    *result = value;
    ret = true;

cleanup:
    // ====================================================================
    // 第 3 步：清理操作数栈
    // ====================================================================

    Linked_List_Destroy(&val_stack);
    return ret;
}

// ==========================================================================
// 演示：中缀表达式 → 后缀表达式 → 求值
//
// 使用示例表达式 ((15/(7-(1+1)))*3)-(2+(1+1))
// 预期后缀：15 7 1 1 + - / 3 * 2 1 1 + + -
// 预期结果：5
//
// 用法：
//   ./out/example_stack_for_expr_conversion_and_evaluation        默认：仅主表达式
//   ./out/example_stack_for_expr_conversion_and_evaluation -t     同时测试错误处理
//
// 流程图解：
//
//   ┌─────────┐    to_postfix()    ┌─────────┐    eval_postfix()    ┌─────────┐
//   │ 中缀 expr │ ─────────────────→ │ 后缀 RPN │ ──────────────────→ │  结果 5  │
//   └─────────┘                     └─────────┘                     └─────────┘
//                                   ↖ Arena 托管内存，一次 free()
// ==========================================================================

// ==========================================================================
// test_errors —— 错误处理健壮性测试
//
// 测试 to_postfix 和 eval_postfix 对各种非法输入的错误检测能力。
// 每个测试用例调用被测函数并检查其返回值：
//   - to_postfix 应返回 NULL 表示检测到错误
//   - eval_postfix 应返回 false 表示检测到错误
// 被测函数自身会通过 printf 输出诊断消息，此处仅验证返回值。
//
// @param arena  用于 to_postfix 内存分配的 Arena 指针
// @return       0 表示全部测试通过，非 0 表示有测试失败（失败计数）
// ==========================================================================

static int test_errors(Arena *arena)
{
    int err_count = 0; // 错误测试中自身未通过的计数
    int pass_count = 0;

    printf("\n=======================================\n");
    printf("  错误处理健壮性测试\n");
    printf("=======================================\n");

    // ----------------------------------------------------------------
    // to_postfix 错误检测
    //
    // 以下输入均为非法中缀表达式，to_postfix 应返回 NULL。
    // ----------------------------------------------------------------
    printf("\n── to_postfix 错误检测 ──\n\n");

    // 用于记录单次测试结果的宏
#define CHECK_TP(expr_input, desc)                                             \
    do                                                                         \
    {                                                                          \
        printf("  %-35s ", desc);                                              \
        char *pfx = to_postfix(arena, expr_input);                             \
        if (pfx == NULL)                                                       \
        {                                                                      \
            printf("✓ 正确拒绝\n");                                            \
            pass_count++;                                                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf("✗ 应返回 NULL，实际: \"%s\"\n", pfx);                      \
            err_count++;                                                       \
        }                                                                      \
    } while (0)

    // to_postfix 错误用例
    CHECK_TP("1+a", "非法字符 'a'");
    CHECK_TP("", "空字符串");
    CHECK_TP("   ", "全空格");
    CHECK_TP("(1+2", "未匹配左括号 '('");
    CHECK_TP("1+2)", "未匹配右括号 ')'");
    CHECK_TP("1@2", "非法字符 '@'");

#undef CHECK_TP

    // ----------------------------------------------------------------
    // eval_postfix 错误检测
    //
    // 以下输入均为非法后缀表达式，eval_postfix 应返回 false。
    // ----------------------------------------------------------------
    printf("\n── eval_postfix 错误检测 ──\n\n");

#define CHECK_EP(postfix_input, desc)                                          \
    do                                                                         \
    {                                                                          \
        printf("  %-35s ", desc);                                              \
        int val;                                                               \
        if (!eval_postfix(postfix_input, &val))                                \
        {                                                                      \
            printf("✓ 正确拒绝\n");                                            \
            pass_count++;                                                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf("✗ 应返回 false，实际结果: %d\n", val);                     \
            err_count++;                                                       \
        }                                                                      \
    } while (0)

    // eval_postfix 错误用例
    CHECK_EP("", "空字符串");
    CHECK_EP("a", "非法字符 'a'");
    CHECK_EP("+", "栈下溢（运算符无操作数）");
    CHECK_EP("1 0 /", "除数为零");
    CHECK_EP("1 2 3 +", "操作数过多");

    // 可检测：NULL 参数（无诊断消息输出，以避免触发 printf 中的 NULL
    // 格式化未定义行为）
    {
        printf("  %-35s ", "result 为 NULL");
        if (!eval_postfix("1", NULL))
        {
            printf("✓ 正确拒绝\n");
            pass_count++;
        }
        else
        {
            printf("✗ 应返回 false\n");
            err_count++;
        }
    }

#undef CHECK_EP

    // ---- 测试汇总 ----
    printf("\n───────────────────────────────────────\n");
    printf("  错误测试结果: %d 通过, %d 失败\n", pass_count, err_count);
    printf("───────────────────────────────────────\n");

    return err_count;
}

int main(int argc, char *argv[])
{
    // 使用 getopt 解析命令行参数
    // -t: 启用错误处理健壮性测试
    bool do_test = false;
    int opt;
    while ((opt = getopt(argc, argv, "t")) != -1)
    {
        switch (opt)
        {
        case 't':
            do_test = true;
            break;
        default:
            printf("用法: %s [-t]\n", argv[0]);
            return 1;
        }
    }

    // ret: 返回值，默认为 1（失败），成功时改为 0
    int ret = 1;

    // ---- 待求值的中缀表达式 ----
    const char *infix = "((15/(7-(1+1)))*3)-(2+(1+1))";

    printf("=======================================\n");
    printf("  表达式转换与求值（调度场算法 + 后缀求值）\n");
    printf("=======================================\n\n");

    // ====================================================================
    // 第 1 步：初始化 Arena 分配器
    //
    // Arena = {0} 将 begin 和 end 指针置为 NULL（零值初始化），
    // 表示此刻 Arena 尚未分配任何 Region。
    // 首次调用 arena_alloc 或 arena_sb_append_buf 时，
    // Arena 内部会自动调用 new_region 分配第一个 8KB Region。
    //
    // 无需手动 Initialize / Destroy，Arena 是纯值类型。
    // ====================================================================
    Arena arena = {0};

    printf("中缀表达式: %s\n\n", infix);

    // ====================================================================
    // 第 2 步：中缀 → 后缀（调度场算法）
    // ====================================================================
    char *postfix = to_postfix(&arena, infix);
    if (postfix == NULL)
    {
        printf("转换失败\n");
        goto cleanup;
    }
    printf("后缀表达式: %s\n\n", postfix);

    // ====================================================================
    // 第 3 步：后缀表达式的值
    // ====================================================================
    int result;
    if (!eval_postfix(postfix, &result))
    {
        printf("后缀求值失败\n");
        goto cleanup;
    }
    printf("求值结果: %d\n", result);

    // ---- 验证预期结果 ----
    printf("\n=======================================\n");
    printf("  预期: ((15/(7-(1+1)))*3)-(2+(1+1)) = 5\n");
    if (result == 5)
    {
        printf("  结果正确 ✓\n");
        ret = 0;
    }
    else
    {
        printf("  结果错误 ✗\n");
    }
    printf("=======================================\n");

    // ====================================================================
    // 错误处理健壮性测试（仅在 -t 标志时启用）
    // ====================================================================
    if (do_test)
    {
        if (test_errors(&arena) != 0)
        {
            ret = 1;
        }
    }

cleanup:
    // ====================================================================
    // 第 4 步：释放 Arena
    //
    // arena_free 一次性释放 postfix 字符串占用的内存以及 Arena
    // 内部所有 Region，无需逐块 free。之后 Arena 所有字段归零。
    // ====================================================================
    arena_free(&arena);

    return ret;
}
