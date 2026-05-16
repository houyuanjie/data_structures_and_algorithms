#include "linked.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// ==========================================================================
// to_postfix —— 调度场算法（Shunting Yard Algorithm）
//
// 将中缀表达式转换为后缀表达式（逆波兰表示法，RPN）。
// 使用双向链表 (Linked_List) 作为运算符栈。
//
// 算法步骤：
//   第 1 步：遍历输入字符串，识别 token（操作数 / 运算符 / 括号）
//   第 2 步：按调度场算法处理每个 token：
//     (a) 操作数 → 直接输出到结果串
//     (b) '('    → 直接入栈
//     (c) ')'    → 弹栈输出直到遇到 '('，丢弃括号对
//     (d) 运算符 → 弹栈中所有优先级 >= 当前运算符的运算符并输出，
//                  然后将当前运算符入栈
//   第 3 步：输入结束后，将栈中剩余运算符全部弹栈输出
//
// @param expr   [const char *] 中缀算术表达式，支持多位数操作数
// @return       [char *] 动态分配的后缀表达式字符串，调用者负责释放
//               NULL 表示内存分配失败
//
// 时间复杂度：O(n) —— 每个 token 均入栈、出栈各至多一次
// 空间复杂度：O(n) —— 运算符栈和输出字符串的大小与输入规模成线性关系
// ==========================================================================

char *to_postfix(const char *expr)
{
    // ---- 第 0 步：初始化 ----

    // 运算符栈（栈顶 ≡ 表头）
    Linked_List op_stack;
    Linked_List_Initialize(&op_stack);

    // 输出缓冲区（后缀表达式字符串）
    size_t out_cap = 4096;
    char *postfix = (char *)malloc(out_cap);
    if (postfix == NULL)
    {
        printf("<to_postfix> 内存分配失败\n");
        Linked_List_Destroy(&op_stack);
        return NULL;
    }
    char *out = postfix;
    char *out_end = postfix + out_cap;
    bool first_token = true; // 首个 token 不加前导空格

    // ---- 第 1 步：扫描输入，按调度场算法处理 ----

    int i = 0;
    while (expr[i] != '\0')
    {
        char c = expr[i];

        // 跳过空白字符
        if (isspace((unsigned char)c))
        {
            i++;
            continue;
        }

        if (isdigit((unsigned char)c))
        {
            // 操作数：读取连续数字（支持多位数）
            int num = 0;
            while (isdigit((unsigned char)expr[i]))
            {
                num = num * 10 + (expr[i] - '0');
                i++;
            }
            // 此时 i 已指向非数字字符，无需额外 i++

            int n;
            if (first_token)
            {
                n = snprintf(out, out_end - out, "%d", num);
                first_token = false;
            }
            else
            {
                n = snprintf(out, out_end - out, " %d", num);
            }
            out += n;
        }
        else
        {
            // 单字符 token：运算符或括号
            char token = c;
            i++;

            if (token == '(')
            {
                // '(' 总是直接入栈
                // 作为右括号弹栈的边界标记
                Linked_Stack_Push(&op_stack, (int)'(');
            }
            else if (token == ')')
            {
                // ')' 连续弹栈直到遇到 '(' 并丢弃它
                //
                //   [栈]  (  op1  op2
                //                ↑ 弹出 op2、op1 输出，丢弃 '('
                int top;
                while (!Linked_List_IsEmpty(&op_stack))
                {
                    Linked_Stack_Peek(&op_stack, &top);
                    if (top == '(')
                    {
                        break;
                    }
                    Linked_Stack_Pop(&op_stack, &top);
                    int n =
                        snprintf(out, out_end - out, " %c", (char)top);
                    out += n;
                }
                // 弹出并丢弃 '('
                if (!Linked_List_IsEmpty(&op_stack))
                {
                    Linked_Stack_Pop(&op_stack, &top);
                }
            }
            else
            {
                // 二元运算符：+ - * /
                int prec = precedence(token);

                // 弹栈条件（满足任一即停止）：
                //   (a) 栈空
                //   (b) 栈顶为 '('
                //   (c) 栈顶运算符优先级 < 当前运算符优先级
                //
                //   while (栈非空 && 栈顶 != '(' && 栈顶优先级 >= 当前优先级)
                //       弹栈输出
                //
                //   先以 IsEmpty 检查栈空，避免 Peek 在空栈时打印诊断消息
                int top;
                while (!Linked_List_IsEmpty(&op_stack))
                {
                    Linked_Stack_Peek(&op_stack, &top);
                    if (top == '(')
                    {
                        break;
                    }
                    if (precedence((char)top) < prec)
                    {
                        break;
                    }
                    Linked_Stack_Pop(&op_stack, &top);
                    int n =
                        snprintf(out, out_end - out, " %c", (char)top);
                    out += n;
                }

                // 当前运算符入栈
                Linked_Stack_Push(&op_stack, (int)token);
            }
        }
    }

    // ---- 第 2 步：清空栈 ----
    // 输入已读完，将栈中剩余运算符全部弹出
    // 先以 IsEmpty 检查栈空，避免 Pop 在空栈时打印诊断消息
    int top;
    while (!Linked_List_IsEmpty(&op_stack))
    {
        Linked_Stack_Pop(&op_stack, &top);
        int n = snprintf(out, out_end - out, " %c", (char)top);
        out += n;
    }

    // ---- 第 3 步：清理 ----
    Linked_List_Destroy(&op_stack);
    return postfix;
}

// ==========================================================================
// eval_postfix —— 后缀表达式求值
//
// 对后缀表达式（逆波兰表示法）求值，返回计算结果。
// 使用双向链表 (Linked_List) 作为操作数栈。
//
// 算法步骤：
//   第 1 步：从左到右扫描后缀表达式 token（空格分隔）
//   第 2 步：对每个 token：
//     (a) 操作数 → 转换为整数后压栈
//     (b) 运算符 → 弹出栈顶两个操作数（先弹右后弹左），
//                  计算后压回栈
//   第 3 步：扫描结束，栈顶即为最终结果
//
// 注意：运算符弹出操作数的顺序是「先右后左」（先弹的是右操作数），
//       这对加法和乘法无影响，但对减法和除法至关重要。
//
// @param postfix [const char *] 以空格分隔的后缀表达式
// @return       [long] 计算结果
//
// 时间复杂度：O(n) —— 每个 token 处理一次
// 空间复杂度：O(n) —— 操作数栈的大小
// ==========================================================================

long eval_postfix(const char *postfix)
{
    // ---- 第 0 步：初始化 ----
    Linked_List val_stack;
    Linked_List_Initialize(&val_stack);

    // ---- 第 1 步：扫描 token 并求值 ----
    const char *p = postfix;
    while (*p != '\0')
    {
        // 跳过空格
        while (*p == ' ')
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        // 判断 token 类型：
        // 运算符为单个字符（+ - * /）且后跟空格或位于结尾
        if (((*p) == '+' || (*p) == '-' || (*p) == '*' || (*p) == '/')
            && (*(p + 1) == ' ' || *(p + 1) == '\0'))
        {
            // 运算符：弹出右操作数和左操作数（顺序很重要！先弹右后弹左）
            char op = *p;
            p++;

            int right, left;
            Linked_Stack_Pop(&val_stack, &right);
            Linked_Stack_Pop(&val_stack, &left);

            long result;
            switch (op)
            {
            case '+':
                result = (long)left + (long)right;
                break;
            case '-':
                result = (long)left - (long)right;
                break;
            case '*':
                result = (long)left * (long)right;
                break;
            case '/':
                result = (long)left / (long)right;
                break;
            default:
                result = 0;
                break;
            }

            Linked_Stack_Push(&val_stack, (int)result);
        }
        else
        {
            // 操作数：转换为整数后压栈
            char *end;
            long num = strtol(p, &end, 10);
            Linked_Stack_Push(&val_stack, (int)num);
            p = end;
        }
    }

    // ---- 第 2 步：弹出最终结果 ----
    int result;
    Linked_Stack_Pop(&val_stack, &result);

    // ---- 第 3 步：清理 ----
    Linked_List_Destroy(&val_stack);
    return (long)result;
}

// ==========================================================================
// 演示：中缀表达式 → 后缀表达式 → 求值
//
// 使用示例表达式 ((15/(7-(1+1)))*3)-(2+(1+1))
// 预期后缀：15 7 1 1 + - / 3 * 2 1 1 + + -
// 预期结果：5
// ==========================================================================

int main()
{
    const char *infix = "((15/(7-(1+1)))*3)-(2+(1+1))";

    printf("=======================================\n");
    printf("  表达式转换与求值（调度场算法 + 后缀求值）\n");
    printf("=======================================\n\n");

    // ---- 第 1 步：中缀 → 后缀 ----
    printf("中缀表达式: %s\n\n", infix);

    char *postfix = to_postfix(infix);
    if (postfix == NULL)
    {
        printf("转换失败\n");
        return 1;
    }
    printf("后缀表达式: %s\n\n", postfix);

    // ---- 第 2 步：后缀求值 ----
    long result = eval_postfix(postfix);
    printf("求值结果: %ld\n", result);

    // ---- 清理 ----
    free(postfix);

    // ---- 验证 ----
    printf("\n=======================================\n");
    printf("  预期: ((15/(7-(1+1)))*3)-(2+(1+1)) = 5\n");
    if (result == 5)
    {
        printf("  结果正确 ✓\n");
    }
    else
    {
        printf("  结果错误 ✗\n");
    }
    printf("=======================================\n");

    return (result == 5) ? 0 : 1;
}
