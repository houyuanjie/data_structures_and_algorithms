#include "linked.h"
#include <stdio.h>

// 测试宏：检查布尔表达式并打印结果
#define TEST(name, expr)                                                       \
    do                                                                         \
    {                                                                          \
        printf("  [测试] %-40s ", name);                                       \
        if (expr)                                                              \
        {                                                                      \
            printf("✓ 通过\n");                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf("✗ 失败 (第 %d 行)\n", __LINE__);                           \
            return 1;                                                          \
        }                                                                      \
    } while (0)

// ==========================================================================
// 基本操作测试
//
// 验证 Initialize → InsertAtOrd → GetElem → PutElem → GetOrdOfElem
// → DeleteAtOrd → Destroy 的完整 CRUD 流程。
// ==========================================================================

int test_basic_operations()
{
    printf("\n=== 基本操作测试 ===\n");

    Linked_List list;
    int elem;

    // 初始化
    TEST("初始化双向链表", Linked_List_Initialize(&list));
    TEST("初始长度为 0", Linked_List_GetLength(&list) == 0);
    TEST("初始为空表", Linked_List_IsEmpty(&list));

    // 插入测试：表头、表尾、中间
    TEST("在位序 1 插入 10", Linked_List_InsertAtOrd(&list, 1, 10));
    TEST("在位序 2 插入 20", Linked_List_InsertAtOrd(&list, 2, 20));
    TEST("在位序 1 插入 5 (表头)", Linked_List_InsertAtOrd(&list, 1, 5));
    TEST("在位序 3 插入 15 (中间)", Linked_List_InsertAtOrd(&list, 3, 15));
    TEST("在位序 5 插入 30 (表尾)", Linked_List_InsertAtOrd(&list, 5, 30));
    TEST("当前长度为 5", Linked_List_GetLength(&list) == 5);

    printf("  当前双向链表内容:\n");
    Linked_List_Print(&list);

    // 获取测试：按位序取元素
    TEST("获取位序 1 的元素", Linked_List_GetElem(&list, 1, &elem));
    TEST("位序 1 的元素是 5", elem == 5);

    TEST("获取位序 2 的元素", Linked_List_GetElem(&list, 2, &elem));
    TEST("位序 2 的元素是 10", elem == 10);

    TEST("获取位序 3 的元素", Linked_List_GetElem(&list, 3, &elem));
    TEST("位序 3 的元素是 15", elem == 15);

    TEST("获取位序 4 的元素", Linked_List_GetElem(&list, 4, &elem));
    TEST("位序 4 的元素是 20", elem == 20);

    TEST("获取位序 5 的元素", Linked_List_GetElem(&list, 5, &elem));
    TEST("位序 5 的元素是 30", elem == 30);

    // 修改测试
    int oldElem;
    TEST("修改位序 2 的元素为 100",
         Linked_List_PutElem(&list, 2, 100, &oldElem));
    TEST("旧值为 10", oldElem == 10);
    TEST("获取修改后的值", Linked_List_GetElem(&list, 2, &elem));
    TEST("新值为 100", elem == 100);

    // 查找测试
    TEST("查找 5 的位序为 1", Linked_List_GetOrdOfElem(&list, 5) == 1);
    TEST("查找 15 的位序为 3", Linked_List_GetOrdOfElem(&list, 15) == 3);
    TEST("查找 30 的位序为 5", Linked_List_GetOrdOfElem(&list, 30) == 5);
    TEST("查找不存在的元素返回 0", Linked_List_GetOrdOfElem(&list, 999) == 0);

    // 删除测试：表头、中间、表尾
    TEST("删除位序 1 (表头) 的元素", Linked_List_DeleteAtOrd(&list, 1, &elem));
    TEST("删除的元素是 5", elem == 5);
    TEST("删除后长度为 4", Linked_List_GetLength(&list) == 4);
    TEST("新表头是 100", Linked_List_GetElem(&list, 1, &elem) && elem == 100);

    TEST("删除位序 3 (中间) 的元素", Linked_List_DeleteAtOrd(&list, 3, &elem));
    TEST("删除的元素是 20", elem == 20);
    TEST("删除后长度为 3", Linked_List_GetLength(&list) == 3);

    TEST("删除位序 3 (表尾) 的元素", Linked_List_DeleteAtOrd(&list, 3, &elem));
    TEST("删除的元素是 30", elem == 30);
    TEST("删除后长度为 2", Linked_List_GetLength(&list) == 2);

    printf("  删除操作后的链表:\n");
    Linked_List_Print(&list);

    // 销毁
    TEST("销毁双向链表", Linked_List_Destroy(&list));
    TEST("销毁后为空表", Linked_List_IsEmpty(&list));

    return 0;
}

// ==========================================================================
// 边界条件测试
//
// 验证 NULL 指针、ord 越界、空表/单元素操作、可选参数为 NULL 等情况。
// ==========================================================================

int test_boundary_conditions()
{
    printf("\n=== 边界条件测试 ===\n");

    Linked_List list;
    int elem;

    Linked_List_Initialize(&list);

    // 越界插入
    TEST("在位序 0 插入 (越界，应失败)", !Linked_List_InsertAtOrd(&list, 0, 1));
    TEST("在位序 2 插入空表 (越界，应失败)",
         !Linked_List_InsertAtOrd(&list, 2, 1));

    // 插入一个元素后测试越界
    Linked_List_InsertAtOrd(&list, 1, 100);
    TEST("在位序 0 插入 (越界，应失败)", !Linked_List_InsertAtOrd(&list, 0, 1));
    TEST("在位序 3 插入 (越界，应失败)", !Linked_List_InsertAtOrd(&list, 3, 1));

    // 越界获取/修改/删除
    TEST("获取位序 0 (越界，应失败)", !Linked_List_GetElem(&list, 0, &elem));
    TEST("获取位序 2 (越界，应失败)", !Linked_List_GetElem(&list, 2, &elem));
    TEST("修改位序 0 (越界，应失败)", !Linked_List_PutElem(&list, 0, 1, NULL));
    TEST("删除位序 0 (越界，应失败)",
         !Linked_List_DeleteAtOrd(&list, 0, &elem));
    TEST("删除位序 2 (越界，应失败)",
         !Linked_List_DeleteAtOrd(&list, 2, &elem));

    // NULL 指针测试：每个对外函数至少测试一次 NULL 参数
    TEST("NULL 初始化 (应失败)", !Linked_List_Initialize(NULL));
    TEST("NULL 销毁 (应失败)", !Linked_List_Destroy(NULL));
    TEST("NULL 获取长度返回 0", Linked_List_GetLength(NULL) == 0);
    TEST("NULL 为空表", Linked_List_IsEmpty(NULL));
    TEST("NULL 查找返回 0", Linked_List_GetOrdOfElem(NULL, 1) == 0);
    TEST("NULL 插入 (应失败)", !Linked_List_InsertAtOrd(NULL, 1, 1));
    TEST("NULL 获取 (应失败)", !Linked_List_GetElem(NULL, 1, &elem));
    TEST("NULL 修改 (应失败)", !Linked_List_PutElem(NULL, 1, 1, NULL));
    TEST("NULL 删除 (应失败)", !Linked_List_DeleteAtOrd(NULL, 1, &elem));

    // NULL 栈/队列参数
    TEST("NULL Push (应失败)", !Linked_Stack_Push(NULL, 1));
    TEST("NULL Pop (应失败)", !Linked_Stack_Pop(NULL, &elem));
    TEST("NULL Peek (应失败)", !Linked_Stack_Peek(NULL, &elem));
    TEST("NULL Enqueue (应失败)", !Linked_Queue_Enqueue(NULL, 1));
    TEST("NULL Dequeue (应失败)", !Linked_Queue_Dequeue(NULL, &elem));
    TEST("NULL PeekFront (应失败)", !Linked_Queue_PeekFront(NULL, &elem));

    // 空表操作
    Linked_List_Destroy(&list);
    Linked_List_Initialize(&list);
    TEST("空表查找返回 0", Linked_List_GetOrdOfElem(&list, 1) == 0);

    // 可选参数测试（elem / oldElem 可为 NULL）
    TEST("获取元素但 elem 为 NULL (空表越界，应失败)",
         !Linked_List_GetElem(&list, 1, NULL));
    Linked_List_InsertAtOrd(&list, 1, 42);
    TEST("获取元素但 elem 为 NULL (有效位序)",
         Linked_List_GetElem(&list, 1, NULL));
    TEST("修改元素但 oldElem 为 NULL", Linked_List_PutElem(&list, 1, 99, NULL));
    TEST("删除元素但 elem 为 NULL", Linked_List_DeleteAtOrd(&list, 1, NULL));

    // 单元素表操作
    Linked_List_Destroy(&list);
    Linked_List_Initialize(&list);
    Linked_List_InsertAtOrd(&list, 1, 1);
    TEST("单元素时 Head == Tail", list.Head == list.Tail);
    TEST("单元素时 Head->Prev 为 NULL", list.Head->Prev == NULL);
    TEST("单元素时 Head->Next 为 NULL", list.Head->Next == NULL);
    Linked_List_DeleteAtOrd(&list, 1, &elem);
    TEST("删除唯一元素后为空表", Linked_List_IsEmpty(&list));
    TEST("删除唯一元素后 Head 为 NULL", list.Head == NULL);
    TEST("删除唯一元素后 Tail 为 NULL", list.Tail == NULL);

    Linked_List_Destroy(&list);
    return 0;
}

// ==========================================================================
// 栈操作测试
//
// 验证 Push/Pop/Peek 的 LIFO（后进先出）语义，以及与通用操作的交叉验证。
// ==========================================================================

int test_stack_operations()
{
    printf("\n=== 栈操作测试 (LIFO) ===\n");

    Linked_List list;
    int elem;

    Linked_List_Initialize(&list);

    // 入栈
    TEST("Push 10", Linked_Stack_Push(&list, 10));
    TEST("Push 20", Linked_Stack_Push(&list, 20));
    TEST("Push 30", Linked_Stack_Push(&list, 30));
    TEST("栈长度为 3", Linked_List_GetLength(&list) == 3);

    printf("  当前栈内容 (栈顶=表头):\n");
    Linked_List_Print(&list);

    // 查看栈顶（不出栈）
    TEST("Peek 栈顶", Linked_Stack_Peek(&list, &elem));
    TEST("栈顶是 30", elem == 30);
    TEST("Peek 后长度不变", Linked_List_GetLength(&list) == 3);

    // 出栈验证 LIFO
    TEST("Pop 第 1 次", Linked_Stack_Pop(&list, &elem));
    TEST("弹出 30", elem == 30);
    TEST("Pop 第 2 次", Linked_Stack_Pop(&list, &elem));
    TEST("弹出 20", elem == 20);
    TEST("Pop 第 3 次", Linked_Stack_Pop(&list, &elem));
    TEST("弹出 10", elem == 10);
    TEST("栈为空", Linked_List_IsEmpty(&list));

    // 空栈操作
    TEST("空栈 Peek (应失败)", !Linked_Stack_Peek(&list, &elem));
    TEST("空栈 Pop (应失败)", !Linked_Stack_Pop(&list, &elem));

    // 栈与通用操作交叉
    Linked_Stack_Push(&list, 100);
    TEST("Push 后通用获取位序 1", Linked_List_GetElem(&list, 1, &elem));
    TEST("通用获取位序 1 是 100", elem == 100);
    Linked_List_InsertAtOrd(&list, 2, 200);
    TEST("通用插入后栈顶仍是 100",
         Linked_Stack_Peek(&list, &elem) && elem == 100);
    TEST("Pop 弹出 100", Linked_Stack_Pop(&list, &elem) && elem == 100);
    TEST("新栈顶是 200", Linked_Stack_Peek(&list, &elem) && elem == 200);

    Linked_List_Destroy(&list);
    return 0;
}

// ==========================================================================
// 队列操作测试
//
// 验证 Enqueue/Dequeue/PeekFront 的 FIFO（先进先出）语义。
// ==========================================================================

int test_queue_operations()
{
    printf("\n=== 队列操作测试 (FIFO) ===\n");

    Linked_List list;
    int elem;

    Linked_List_Initialize(&list);

    // 入队
    TEST("Enqueue 10", Linked_Queue_Enqueue(&list, 10));
    TEST("Enqueue 20", Linked_Queue_Enqueue(&list, 20));
    TEST("Enqueue 30", Linked_Queue_Enqueue(&list, 30));
    TEST("队列长度为 3", Linked_List_GetLength(&list) == 3);

    printf("  当前队列内容 (队首=表头, 队尾=表尾):\n");
    Linked_List_Print(&list);

    // 查看队首（不出队）
    TEST("PeekFront 队首", Linked_Queue_PeekFront(&list, &elem));
    TEST("队首是 10", elem == 10);
    TEST("PeekFront 后长度不变", Linked_List_GetLength(&list) == 3);

    // 出队验证 FIFO
    TEST("Dequeue 第 1 次", Linked_Queue_Dequeue(&list, &elem));
    TEST("出队 10", elem == 10);
    TEST("Dequeue 第 2 次", Linked_Queue_Dequeue(&list, &elem));
    TEST("出队 20", elem == 20);
    TEST("Dequeue 第 3 次", Linked_Queue_Dequeue(&list, &elem));
    TEST("出队 30", elem == 30);
    TEST("队列为空", Linked_List_IsEmpty(&list));

    // 空队操作
    TEST("空队 PeekFront (应失败)", !Linked_Queue_PeekFront(&list, &elem));
    TEST("空队 Dequeue (应失败)", !Linked_Queue_Dequeue(&list, &elem));

    // 队列与通用操作交叉
    Linked_Queue_Enqueue(&list, 100);
    Linked_Queue_Enqueue(&list, 200);
    TEST("通用获取位序 2 (队尾)", Linked_List_GetElem(&list, 2, &elem));
    TEST("位序 2 是 200", elem == 200);
    Linked_List_InsertAtOrd(&list, 2, 150);
    TEST("中间插入后长度为 3", Linked_List_GetLength(&list) == 3);
    TEST("队首仍为 100", Linked_Queue_PeekFront(&list, &elem) && elem == 100);
    // 出队：100, 150, 200
    TEST("Dequeue 出 100", Linked_Queue_Dequeue(&list, &elem) && elem == 100);
    TEST("Dequeue 出 150", Linked_Queue_Dequeue(&list, &elem) && elem == 150);
    TEST("Dequeue 出 200", Linked_Queue_Dequeue(&list, &elem) && elem == 200);

    Linked_List_Destroy(&list);
    return 0;
}

// ==========================================================================
// 综合场景测试
//
// 验证双向性（Prev 指针正确）、栈队列混合使用、反复头尾中间操作等。
// ==========================================================================

int test_complex_scenarios()
{
    printf("\n=== 综合场景测试 ===\n");

    Linked_List list;
    int elem;

    Linked_List_Initialize(&list);

    // 场景 1：Prev 指针双向性验证
    printf("  场景 1：双向性验证\n");
    for (int i = 1; i <= 5; i++)
    {
        Linked_List_InsertAtOrd(&list, i, i * 10);
    }
    // 从 Head 沿 Next 前进，再从 Tail 沿 Prev 后退，值应对称
    Linked_Node *forward = list.Head;
    Linked_Node *backward = list.Tail;
    int forward_vals[5], backward_vals[5];
    for (int i = 0; i < 5; i++)
    {
        forward_vals[i] = forward->Value;
        backward_vals[4 - i] = backward->Value;
        forward = forward->Next;
        backward = backward->Prev;
    }
    int same = 1;
    for (int i = 0; i < 5; i++)
    {
        if (forward_vals[i] != backward_vals[i])
        {
            same = 0;
            break;
        }
    }
    TEST("正反向遍历值一致", same);
    TEST("正向遍历后 forward 为 NULL", forward == NULL);
    TEST("反向遍历后 backward 为 NULL", backward == NULL);

    // 场景 2：表头反复插入
    printf("  场景 2：表头反复插入\n");
    Linked_List_Destroy(&list);
    Linked_List_Initialize(&list);
    for (int ord = 1; ord <= 5; ord++)
    {
        Linked_List_InsertAtOrd(&list, 1, ord);
    }
    // 结果应为 5, 4, 3, 2, 1
    TEST("头部插入后长度为 5", Linked_List_GetLength(&list) == 5);
    Linked_List_GetElem(&list, 1, &elem);
    TEST("第 1 个是 5", elem == 5);
    Linked_List_GetElem(&list, 5, &elem);
    TEST("第 5 个是 1", elem == 1);

    // 场景 3：表尾反复追加
    printf("  场景 3：表尾反复追加\n");
    int len = Linked_List_GetLength(&list);
    Linked_List_InsertAtOrd(&list, len + 1, 100);
    Linked_List_InsertAtOrd(&list, len + 2, 200);
    Linked_List_InsertAtOrd(&list, len + 3, 300);
    TEST("尾部追加后长度为 8", Linked_List_GetLength(&list) == 8);

    // 场景 4：中间插入
    printf("  场景 4：在中间位序插入\n");
    Linked_List_InsertAtOrd(&list, 4, 999);
    Linked_List_GetElem(&list, 3, &elem);
    TEST("第 3 个是 3", elem == 3);
    Linked_List_GetElem(&list, 4, &elem);
    TEST("第 4 个是 999", elem == 999);
    Linked_List_GetElem(&list, 5, &elem);
    TEST("第 5 个是 2", elem == 2);

    printf("  操作后的链表:\n");
    Linked_List_Print(&list);

    // 场景 5：删除头部、尾部、中间
    printf("  场景 5：删除头部/尾部/中间\n");
    Linked_List_DeleteAtOrd(&list, 1, &elem);
    TEST("删除头部元素是 5", elem == 5);
    TEST("新头部是 4", Linked_List_GetElem(&list, 1, &elem) && elem == 4);

    int old_len = Linked_List_GetLength(&list);
    Linked_List_DeleteAtOrd(&list, old_len, &elem);
    TEST("删除尾部元素是 300", elem == 300);

    // 删除头部后 999 已从位序 4 移到 3，用查找定位
    int ord999 = Linked_List_GetOrdOfElem(&list, 999);
    Linked_List_DeleteAtOrd(&list, ord999, &elem);
    TEST("删除中间元素是 999", elem == 999);

    printf("  删除操作后的链表:\n");
    Linked_List_Print(&list);

    // 场景 6：栈队列混合使用
    printf("  场景 6：栈队列混合使用\n");
    Linked_List_Destroy(&list);
    Linked_List_Initialize(&list);

    // 用栈操作放入 [A, B, C]（栈顶 B, C, A 的顺序? No）
    // Push A: 栈 [A]
    // Push B: 栈 [B, A]
    // Push C: 栈 [C, B, A]
    Linked_Stack_Push(&list, 10); // 栈: [10]
    Linked_Stack_Push(&list, 20); // 栈: [20, 10]
    Linked_Stack_Push(&list, 30); // 栈: [30, 20, 10]

    TEST("栈中有 3 个元素", Linked_List_GetLength(&list) == 3);

    // 用队列操作出队（FIFO，队首=表头，即栈顶）
    // 当前表头是 30，所以 Dequeue 应该出 30
    TEST("Dequeue 出 30", Linked_Queue_Dequeue(&list, &elem) && elem == 30);
    TEST("Dequeue 出 20", Linked_Queue_Dequeue(&list, &elem) && elem == 20);
    TEST("Dequeue 出 10", Linked_Queue_Dequeue(&list, &elem) && elem == 10);
    TEST("队列为空", Linked_List_IsEmpty(&list));

    // 反过来：用 Enqueue 入队，用 Pop 出栈
    Linked_Queue_Enqueue(&list, 1); // 队: [1]
    Linked_Queue_Enqueue(&list, 2); // 队: [1, 2]
    Linked_Queue_Enqueue(&list, 3); // 队: [1, 2, 3]

    // Pop 从栈顶（表头）出，LIFO
    TEST("Pop 出 1", Linked_Stack_Pop(&list, &elem) && elem == 1);
    TEST("Pop 出 2", Linked_Stack_Pop(&list, &elem) && elem == 2);
    TEST("Pop 出 3", Linked_Stack_Pop(&list, &elem) && elem == 3);

    // 场景 7：大量元素插入删除（压力测试）
    printf("  场景 7：100 个元素的插入和删除\n");
    for (int i = 1; i <= 100; i++)
    {
        if (!Linked_List_InsertAtOrd(&list, i, i * 3))
        {
            printf("  插入第 %d 个元素失败\n", i);
            return 1;
        }
    }
    TEST("插入 100 个元素后长度为 100", Linked_List_GetLength(&list) == 100);
    // 验证第 1、50、100 个元素
    Linked_List_GetElem(&list, 1, &elem);
    TEST("第 1 个元素是 3", elem == 3);
    Linked_List_GetElem(&list, 50, &elem);
    TEST("第 50 个元素是 150", elem == 150);
    Linked_List_GetElem(&list, 100, &elem);
    TEST("第 100 个元素是 300", elem == 300);

    // 逆序删除
    for (int i = 100; i >= 1; i--)
    {
        Linked_List_DeleteAtOrd(&list, i, &elem);
    }
    TEST("逆序删除后为空表", Linked_List_IsEmpty(&list));

    Linked_List_Destroy(&list);
    return 0;
}

int main()
{
    printf("=======================================\n");
    printf("     双向链表 (Linked_List) 测试程序   \n");
    printf("=======================================\n");

    int result = 0;

    result |= test_basic_operations();
    result |= test_boundary_conditions();
    result |= test_stack_operations();
    result |= test_queue_operations();
    result |= test_complex_scenarios();

    printf("\n=======================================\n");
    if (result == 0)
    {
        printf("       所有测试通过！✓               \n");
    }
    else
    {
        printf("       存在失败的测试 ✗              \n");
    }
    printf("=======================================\n");

    return result;
}
