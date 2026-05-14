#include "linked.h"

#include <stdio.h>
#include <stdlib.h>

// ==========================================================================
// 双向链表 (Linked_List) 实现
//
// 所有对外函数在遇到错误时均会打印带函数名的诊断消息，然后返回 false 或 0，
// 这使得调用方可以简单地用 if (!Func(...)) 判断失败，无需管理错误码。
//
// 内部约定：
//   - ord 一律表示位序（1-based），来自外部调用者
//   - i / j / k 一律表示遍历计数（1-based 递增到 ord），仅在实现内部使用
//   - ord = i 时到达目标结点
//   - 有效性检查的顺序：先检查 list 是否为 NULL → 再检查 ord 越界
//     （空表时 Head 为 NULL 是合法状态，由 ord 越界检查负责拦截非法访问）
// ==========================================================================

// ==========================================================================
// 内部辅助函数：按位序遍历寻址
//
// 从表头出发，沿 Next 指针前进 ord-1 步到达目标结点并返回其指针。
// 前提：调用者已确保 list 非 NULL、已初始化、ord 在有效范围 [1, Length] 内。
//
// 时间复杂度：O(n)（最坏遍历到表尾）
// ==========================================================================

static Linked_Node *_Linked_List_GetNodeAt(Linked_List *list, int ord)
{
    Linked_Node *node = list->Head;
    // i 从 1 递增到 ord：每步沿 Next 前进一格
    for (int i = 1; i < ord; i++)
    {
        node = node->Next;
    }
    return node;
}

// ==========================================================================
// 创建与销毁
// ==========================================================================

bool Linked_List_Initialize(Linked_List *list)
{
    if (list == NULL)
    {
        printf("<Linked_List_Initialize> 参数 list 为 NULL，无法初始化\n");
        return false;
    }

    list->Head = NULL;
    list->Tail = NULL;
    list->Length = 0;

    return true;
}

bool Linked_List_Destroy(Linked_List *list)
{
    if (list == NULL)
    {
        printf("<Linked_List_Destroy> 参数 list 为 NULL，无需销毁\n");
        return false;
    }

    // 从表头开始，沿 Next 指针逐个释放所有结点
    Linked_Node *node = list->Head;
    while (node != NULL)
    {
        Linked_Node *next = node->Next;
        free(node);
        node = next;
    }

    list->Head = NULL;
    list->Tail = NULL;
    list->Length = 0;

    return true;
}

// ==========================================================================
// 状态查询
// ==========================================================================

int Linked_List_GetLength(Linked_List *list)
{
    if (list == NULL)
    {
        printf("<Linked_List_GetLength> 参数 list 为 NULL，返回 0\n");
        return 0;
    }
    return list->Length;
}

bool Linked_List_IsEmpty(Linked_List *list)
{
    return list == NULL || list->Head == NULL || list->Length == 0;
}

void Linked_List_Print(Linked_List *list)
{
    if (list == NULL || list->Head == NULL)
    {
        printf("(null)\n");
        return;
    }

    if (list->Length == 0)
    {
        printf("[]\n");
        return;
    }

    printf("[\n");
    Linked_Node *node = list->Head;
    for (int ord = 1; node != NULL; ord++, node = node->Next)
    {
        printf("  [%2d] = %d,\n", ord, node->Value);
    }
    printf("]\n");
}

// ==========================================================================
// 元素存取
// ==========================================================================

bool Linked_List_GetElem(Linked_List *list, int ord, int *elem)
{
    if (list == NULL)
    {
        printf("<Linked_List_GetElem> list 为 NULL，无法获取元素\n");
        return false;
    }
    if (ord < 1 || list->Length < ord)
    {
        printf(
            "<Linked_List_GetElem> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
            ord, list->Length, list->Length);
        return false;
    }

    // ord 是位序（1-based），沿 Next 指针前进 ord-1 步到达目标结点
    Linked_Node *node = _Linked_List_GetNodeAt(list, ord);
    if (elem != NULL)
    {
        *elem = node->Value;
    }
    return true;
}

bool Linked_List_PutElem(Linked_List *list, int ord, int elem, int *oldElem)
{
    if (list == NULL)
    {
        printf("<Linked_List_PutElem> list 为 NULL，无法修改元素\n");
        return false;
    }
    if (ord < 1 || list->Length < ord)
    {
        printf(
            "<Linked_List_PutElem> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
            ord, list->Length, list->Length);
        return false;
    }

    // ord 是位序（1-based），沿 Next 指针前进 ord-1 步到达目标结点
    Linked_Node *node = _Linked_List_GetNodeAt(list, ord);
    if (oldElem != NULL)
    {
        *oldElem = node->Value;
    }

    node->Value = elem;

    return true;
}

// ==========================================================================
// 查找 —— 顺序查找算法
//
// 从表头开始沿 Next 指针顺序遍历，逐个比对结点值。
// 时间复杂度 O(n)，空间复杂度 O(1)。
// ==========================================================================

int Linked_List_GetOrdOfElem(Linked_List *list, int elem)
{
    if (list == NULL)
    {
        printf("<Linked_List_GetOrdOfElem> list 为 NULL，无法查找\n");
        return 0;
    }

    if (list->Length == 0)
    {
        printf("<Linked_List_GetOrdOfElem> 表为空，元素 %d 不存在\n", elem);
        return 0;
    }

    // 顺序扫描：从 Head 开始，沿 Next 指针逐个比对
    Linked_Node *node = list->Head;
    for (int ord = 1; node != NULL; ord++, node = node->Next)
    {
        if (node->Value == elem)
        {
            return ord; // 返回位序（1-based）
        }
    }

    printf("<Linked_List_GetOrdOfElem> 元素 %d 未找到\n", elem);
    return 0;
}

// ==========================================================================
// 插入 —— 核心算法
//
// 在指定位序 ord 处插入新元素 elem。根据插入位置分为四种情况：
//
// 情况 1：空表插入 (ord = 1, Length = 0)
//
//   插入前:  Head = NULL,  Tail = NULL
//   插入后:  Head = New →  Tail = New
//            New->Prev = NULL,  New->Next = NULL,  Length = 1
//
// 情况 2：表头插入 (ord = 1, Length ≥ 1)
//
//   插入前:  [新结点?]        Head ⇄ A ⇄ B ⇄ ...
//   插入后:  [新结点=Head] ⇄ 原Head(=A) ⇄ B ⇄ ...
//
//   步骤：
//     1) New->Prev = NULL
//     2) New->Next = 原Head
//     3) 原Head->Prev = New
//     4) Head = New
//     5) Length++
//
// 情况 3：表尾插入 (ord = Length + 1)
//
//   插入前:  ... ⇄ A ⇄ Tail        [新结点?]
//   插入后:  ... ⇄ A ⇄ 原Tail ⇄ [新结点=Tail]
//
//   步骤：
//     1) New->Prev = 原Tail
//     2) New->Next = NULL
//     3) 原Tail->Next = New
//     4) Tail = New
//     5) Length++
//
// 情况 4：中间插入 (1 < ord ≤ Length)
//
//   令 pred = 位序 ord-1 的结点（前驱），succ = pred->Next（后继）
//
//   插入前:  ... ⇄ pred ⇄ succ ⇄ ...
//   插入后:  ... ⇄ pred ⇄ New ⇄ succ ⇄ ...
//
//   步骤：
//     1) New->Prev = pred
//     2) New->Next = succ
//     3) pred->Next = New
//     4) succ->Prev = New
//     5) Length++
//
// 时间复杂度：O(n)（情况 4 需要遍历找到 pred）
// ==========================================================================

bool Linked_List_InsertAtOrd(Linked_List *list, int ord, int elem)
{
    if (list == NULL)
    {
        printf("<Linked_List_InsertAtOrd> list 为 NULL，无法插入\n");
        return false;
    }
    if (ord < 1 || list->Length + 1 < ord)
    {
        printf(
            "<Linked_List_InsertAtOrd> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
            ord, list->Length, list->Length + 1);
        return false;
    }

    // ---- 第 1 步：创建新结点 ----
    Linked_Node *node = (Linked_Node *)malloc(sizeof(Linked_Node));
    if (node == NULL)
    {
        printf("<Linked_List_InsertAtOrd> 内存分配失败\n");
        return false;
    }
    node->Value = elem;

    // ---- 第 2 步：根据位置分情况链接 ----

    if (list->Length == 0)
    {
        // 情况 1：空表插入
        node->Prev = NULL;
        node->Next = NULL;
        list->Head = node;
        list->Tail = node;
    }
    else if (ord == 1)
    {
        // 情况 2：表头插入
        // 新结点成为新的 Head，原 Head 变为第二个结点
        //
        //   [New] → [原Head] → ...
        //    ↑Head     ↑ (原Head->Prev 回指 New)
        node->Prev = NULL;
        node->Next = list->Head;
        list->Head->Prev = node;
        list->Head = node;
    }
    else if (ord == list->Length + 1)
    {
        // 情况 3：表尾插入
        // 新结点成为新的 Tail，追加到原 Tail 之后
        //
        //   ... → [原Tail] → [New]
        //                       ↑Tail
        node->Prev = list->Tail;
        node->Next = NULL;
        list->Tail->Next = node;
        list->Tail = node;
    }
    else
    {
        // 情况 4：中间插入
        // 找到插入位置的前驱结点 pred（位序 ord-1），succ = pred->Next
        //
        //   ... ⇄ pred ⇄  succ  ⇄ ...
        //            ↘   ↗
        //             New
        //
        //   分四步调整指针，顺序必须严格：
        //   ① New 连接前驱  ② New 连接后继  ③ 前驱指向 New  ④ 后继回指 New
        Linked_Node *pred = _Linked_List_GetNodeAt(list, ord - 1);
        Linked_Node *succ = pred->Next;

        node->Prev = pred;
        node->Next = succ;
        pred->Next = node;
        succ->Prev = node;
    }

    list->Length++;
    return true;
}

// ==========================================================================
// 删除 —— 核心算法
//
// 删除位序为 ord 的元素。根据删除位置分为四种情况：
//
// 情况 1：删除唯一结点 (Length = 1, ord = 1)
//
//   删除前:  Head = Tail = node
//   删除后:  Head = NULL, Tail = NULL, Length = 0
//   free(node)
//
// 情况 2：删除表头 (ord = 1, Length > 1)
//
//   删除前:  node(=Head) ⇄ successor ⇄ ...
//   删除后:  successor(=Head) ⇄ ...
//
//   步骤：
//     1) 保存 node（= Head）
//     2) successor = node->Next
//     3) successor->Prev = NULL
//     4) Head = successor
//     5) free(node)
//     6) Length--
//
// 情况 3：删除表尾 (ord = Length, Length > 1)
//
//   删除前:  ... ⇄ predecessor ⇄ node(=Tail)
//   删除后:  ... ⇄ predecessor(=Tail)
//
//   步骤：
//     1) 保存 node（= Tail）
//     2) predecessor = node->Prev
//     3) predecessor->Next = NULL
//     4) Tail = predecessor
//     5) free(node)
//     6) Length--
//
// 情况 4：中间删除 (1 < ord < Length)
//
//   令 node = 位序 ord 的结点（待删除），pred = node->Prev，succ = node->Next
//
//   删除前:  ... ⇄ pred ⇄ node ⇄ succ ⇄ ...
//   删除后:  ... ⇄ pred ⇄ succ ⇄ ...
//
//   步骤：
//     1) pred->Next = succ
//     2) succ->Prev = pred
//     3) free(node)
//     4) Length--
//
// 时间复杂度：O(n)（情况 4 需要遍历找到 node）
// ==========================================================================

bool Linked_List_DeleteAtOrd(Linked_List *list, int ord, int *elem)
{
    if (list == NULL)
    {
        printf("<Linked_List_DeleteAtOrd> list 为 NULL，无法删除\n");
        return false;
    }
    if (ord < 1 || list->Length < ord)
    {
        printf(
            "<Linked_List_DeleteAtOrd> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
            ord, list->Length, list->Length);
        return false;
    }

    // ---- 第 1 步：定位待删除结点 ----

    Linked_Node *node = _Linked_List_GetNodeAt(list, ord);

    // ---- 第 2 步：传出被删除的值 ----

    if (elem != NULL)
    {
        *elem = node->Value;
    }

    // ---- 第 3 步：根据位置分情况解除链接并释放 ----

    if (list->Length == 1)
    {
        // 情况 1：删除唯一结点
        list->Head = NULL;
        list->Tail = NULL;
    }
    else if (ord == 1)
    {
        // 情况 2：删除表头
        //
        //   [node=Head] → [successor] → ...
        //    × 删除        ↑ 成为新 Head
        //
        //   将 successor->Prev 置为 NULL，代表新表头无前驱
        Linked_Node *successor = node->Next;
        successor->Prev = NULL;
        list->Head = successor;
    }
    else if (ord == list->Length)
    {
        // 情况 3：删除表尾
        //
        //   ... → [predecessor] → [node=Tail]
        //             ↑ 成为新 Tail    × 删除
        //
        //   将 predecessor->Next 置为 NULL，代表新表尾无后继
        Linked_Node *predecessor = node->Prev;
        predecessor->Next = NULL;
        list->Tail = predecessor;
    }
    else
    {
        // 情况 4：中间删除
        //
        //   ... ⇄ pred ⇄ node ⇄ succ ⇄ ...
        //             ↘________↗
        //            pred->Next = succ
        //            succ->Prev = pred
        //
        //   让 pred 和 succ 互相指向，绕过待删除的 node
        Linked_Node *pred = node->Prev;
        Linked_Node *succ = node->Next;
        pred->Next = succ;
        succ->Prev = pred;
    }

    free(node);
    list->Length--;

    return true;
}

// ==========================================================================
// 栈操作 —— LIFO（后进先出）
//
// 栈顶 ≡ 表头（双向链表的第一个结点）。
//
// 以下每个栈操作函数均独立实现了完整的算法逻辑，
// 同时在注释中标注其等价于哪些通用链表操作的组合。
// ==========================================================================

// ==========================================================================
// 入栈 —— 在栈顶（表头）插入新元素
//
// 新结点成为新的栈顶（即新的 Head），原 Head 成为其下一个结点。
// 等价于 Linked_List_InsertAtOrd(list, 1, elem)，此处独立展开以展示指针操纵细节。
//
// 算法步骤：
//   第 1 步：创建新结点，Value = elem
//   第 2 步：将新结点链接到链表头部
//     - 空栈时：新结点既是 Head 也是 Tail，Prev/Next 均为 NULL
//     - 非空时：新结点→Next = 原Head，原Head→Prev = 新结点，Head = 新结点
//   第 3 步：Length++
//
//       入栈前:  Top(=Head) → [A] ⇄ [B] ⇄ ...
//       入栈后:  Top(=Head) → [New] ⇄ [A] ⇄ [B] ⇄ ...
//
// 时间复杂度：O(1)
// ==========================================================================

bool Linked_Stack_Push(Linked_List *list, int elem)
{
    if (list == NULL)
    {
        printf("<Linked_Stack_Push> list 为 NULL，无法入栈\n");
        return false;
    }

    // 第 1 步：创建新结点
    Linked_Node *node = (Linked_Node *)malloc(sizeof(Linked_Node));
    if (node == NULL)
    {
        printf("<Linked_Stack_Push> 内存分配失败\n");
        return false;
    }
    node->Value = elem;

    // 第 2 步：链接到表头（栈顶）
    if (list->Length == 0)
    {
        // 空栈：新结点是唯一的元素
        node->Prev = NULL;
        node->Next = NULL;
        list->Head = node;
        list->Tail = node;
    }
    else
    {
        // 非空栈：新结点插入原 Head 之前，成为新的 Head
        node->Prev = NULL;
        node->Next = list->Head;
        list->Head->Prev = node;
        list->Head = node;
    }

    // 第 3 步
    list->Length++;
    return true;
}

// ==========================================================================
// 出栈 —— 从栈顶（表头）移除元素
//
// 移除 Head 结点，返回其值（若 elem 非 NULL），Head 后移。
// 等价于 Linked_List_DeleteAtOrd(list, 1, elem)，此处独立展开以展示指针操纵细节。
//
// 算法步骤：
//   第 1 步：保存栈顶结点（= Head）和其值
//   第 2 步：将 Head 后移一个结点，断开链接
//     - 仅剩一个结点时：Head 和 Tail 均置 NULL
//     - 多个结点时：Head = Head→Next，新 Head→Prev = NULL
//   第 3 步：释放原栈顶结点，Length--
//
//       出栈前:  Top(=Head) → [X] ⇄ [A] ⇄ ...
//                     弹出 ↙
//       出栈后:  Top(=Head) → [A] ⇄ ...
//
// 时间复杂度：O(1)
// ==========================================================================

bool Linked_Stack_Pop(Linked_List *list, int *elem)
{
    if (list == NULL)
    {
        printf("<Linked_Stack_Pop> list 为 NULL，无法出栈\n");
        return false;
    }
    if (list->Length == 0)
    {
        printf("<Linked_Stack_Pop> 栈为空，无法出栈\n");
        return false;
    }

    // 第 1 步：保存栈顶结点和值
    Linked_Node *node = list->Head;
    if (elem != NULL)
    {
        *elem = node->Value;
    }

    // 第 2 步：断开链接，Head 后移
    if (list->Length == 1)
    {
        // 仅剩一个结点：清空 Head 和 Tail
        list->Head = NULL;
        list->Tail = NULL;
    }
    else
    {
        // 多个结点：Head 后移到下一个结点
        Linked_Node *successor = node->Next;
        successor->Prev = NULL;
        list->Head = successor;
    }

    // 第 3 步
    free(node);
    list->Length--;
    return true;
}

// ==========================================================================
// 查看栈顶 —— 获取栈顶（表头）元素的值，不出栈
//
// 等价于 Linked_List_GetElem(list, 1, elem)。
//
// 时间复杂度：O(1)
// ==========================================================================

bool Linked_Stack_Peek(Linked_List *list, int *elem)
{
    if (list == NULL)
    {
        printf("<Linked_Stack_Peek> list 为 NULL，无法查看栈顶\n");
        return false;
    }
    if (list->Length == 0)
    {
        printf("<Linked_Stack_Peek> 栈为空\n");
        return false;
    }

    if (elem != NULL)
    {
        *elem = list->Head->Value;
    }
    return true;
}

// ==========================================================================
// 队列操作 —— FIFO（先进先出）
//
// 队首 ≡ 表头，队尾 ≡ 表尾。
//
// 以下每个队列操作函数均独立实现了完整的算法逻辑。
// ==========================================================================

// ==========================================================================
// 入队 —— 在队尾（表尾）追加新元素
//
// 新结点成为新的队尾（即新的 Tail），原 Tail 成为其前一个结点。
// 等价于 Linked_List_InsertAtOrd(list, list->Length + 1, elem)，此处独立展开。
//
// 算法步骤：
//   第 1 步：创建新结点，Value = elem
//   第 2 步：将新结点链接到表尾（队尾）
//     - 空队时：新结点既是 Head 也是 Tail，Prev/Next 均为 NULL
//     - 非空时：原Tail→Next = 新结点，新结点→Prev = 原Tail，Tail = 新结点
//   第 3 步：Length++
//
//       入队前:  ... ⇄ [A] ⇄ [B] ← Back(=Tail)
//       入队后:  ... ⇄ [A] ⇄ [B] ⇄ [New] ← Back(=Tail)
//
// 时间复杂度：O(1)
// ==========================================================================

bool Linked_Queue_Enqueue(Linked_List *list, int elem)
{
    if (list == NULL)
    {
        printf("<Linked_Queue_Enqueue> list 为 NULL，无法入队\n");
        return false;
    }

    // 第 1 步：创建新结点
    Linked_Node *node = (Linked_Node *)malloc(sizeof(Linked_Node));
    if (node == NULL)
    {
        printf("<Linked_Queue_Enqueue> 内存分配失败\n");
        return false;
    }
    node->Value = elem;

    // 第 2 步：链接到表尾（队尾）
    if (list->Length == 0)
    {
        // 空队：新结点是唯一的元素
        node->Prev = NULL;
        node->Next = NULL;
        list->Head = node;
        list->Tail = node;
    }
    else
    {
        // 非空队：新结点追加到原 Tail 之后，成为新的 Tail
        node->Prev = list->Tail;
        node->Next = NULL;
        list->Tail->Next = node;
        list->Tail = node;
    }

    // 第 3 步
    list->Length++;
    return true;
}

// ==========================================================================
// 出队 —— 从队首（表头）移除元素
//
// 移除 Head 结点（与栈出栈在实现上完全相同——都是从表头删除），
// 但语义不同：队列 FIFO——最先入队的元素最先出队。
// 等价于 Linked_List_DeleteAtOrd(list, 1, elem)，此处独立展开以展示
// 队列 FIFO 的 ADT 语义。
//
// 算法步骤：
//   第 1 步：保存队首结点（= Head）和其值
//   第 2 步：将 Head 后移一个结点，断开链接
//     - 仅剩一个结点时：Head 和 Tail 均置 NULL
//     - 多个结点时：Head = Head→Next，新 Head→Prev = NULL
//   第 3 步：释放原队首结点，Length--
//
//       出队前:  Front(=Head) → [X] ⇄ [B] ⇄ ... ← Back(=Tail)
//                   出队 ↙
//       出队后:  Front(=Head) → [B] ⇄ ... ← Back(=Tail)
//
// 时间复杂度：O(1)
// ==========================================================================

bool Linked_Queue_Dequeue(Linked_List *list, int *elem)
{
    if (list == NULL)
    {
        printf("<Linked_Queue_Dequeue> list 为 NULL，无法出队\n");
        return false;
    }
    if (list->Length == 0)
    {
        printf("<Linked_Queue_Dequeue> 队列为空，无法出队\n");
        return false;
    }

    // 第 1 步：保存队首结点和值
    Linked_Node *node = list->Head;
    if (elem != NULL)
    {
        *elem = node->Value;
    }

    // 第 2 步：断开链接，Head 后移
    if (list->Length == 1)
    {
        // 仅剩一个结点：清空 Head 和 Tail
        list->Head = NULL;
        list->Tail = NULL;
    }
    else
    {
        // 多个结点：Head 后移到下一个结点
        Linked_Node *successor = node->Next;
        successor->Prev = NULL;
        list->Head = successor;
    }

    // 第 3 步
    free(node);
    list->Length--;
    return true;
}

// ==========================================================================
// 查看队首 —— 获取队首（表头）元素的值，不出队
//
// 等价于 Linked_List_GetElem(list, 1, elem)。
//
// 时间复杂度：O(1)
// ==========================================================================

bool Linked_Queue_PeekFront(Linked_List *list, int *elem)
{
    if (list == NULL)
    {
        printf("<Linked_Queue_PeekFront> list 为 NULL，无法查看队首\n");
        return false;
    }
    if (list->Length == 0)
    {
        printf("<Linked_Queue_PeekFront> 队列为空\n");
        return false;
    }

    if (elem != NULL)
    {
        *elem = list->Head->Value;
    }
    return true;
}
