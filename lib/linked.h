#pragma once

#include <stdbool.h>

// ==========================================================================
// 双向链表 (Doubly Linked List) —— 通过操纵 Prev/Next 指针实现
//
// 核心概念：
//   - 双向链表由一系列结点 (Linked_Node) 组成，每个结点包含一个数据域 Value
//     和两个指针域 Prev（前驱）、Next（后继）
//   - 不设哨兵结点：空表的 Head 和 Tail 均为 NULL
//   - 位序 (ord)：逻辑上第几个元素，从 1 开始计数，供外部调用者使用
//   - Length：当前结点个数（表长）
//   - 双向链表可同时作为栈（LIFO，栈顶=表头）和队列（FIFO，队尾进、队首出）使用
//
// 各函数在遇到参数无效等错误时，会打印清晰的错误消息并返回 false 或 0，
// 调用者无需再区分不同的错误码，专注于返回值所代表的成功/失败语义。
// ==========================================================================

// ---- 类型定义 ----

// 双向链表结点
typedef struct Linked_Node
{
    int Value;                // 结点值
    struct Linked_Node *Prev; // 前驱指针（NULL 表示无前驱）
    struct Linked_Node *Next; // 后继指针（NULL 表示无后继）
} Linked_Node;

// 双向链表
typedef struct Linked_List
{
    Linked_Node *Head; // 首结点指针（NULL = 空表）
    Linked_Node *Tail; // 尾结点指针（NULL = 空表）
    int Length;        // 当前结点个数（表长）
} Linked_List;

// ---- 创建与销毁 ----

// 初始化双向链表。将 Head、Tail 置为 NULL，Length 置为 0。
// list 为 NULL 时打印错误并返回 false。
bool Linked_List_Initialize(Linked_List *list);

// 销毁双向链表。从表头开始依次释放所有结点，然后将三个字段重置为 NULL/0。
// list 为 NULL 时打印错误并返回 false。
bool Linked_List_Destroy(Linked_List *list);

// ---- 状态查询 ----

// 获取表长（当前结点个数）。
// list 为 NULL 时打印错误并返回 0。
int Linked_List_GetLength(Linked_List *list);

// 判断是否为空表。
// list 为 NULL、未初始化或 Length == 0 时均返回 true。
bool Linked_List_IsEmpty(Linked_List *list);

// 打印表中的所有元素。
// 从表头开始沿 Next 指针遍历，按位序 (ord) 从 1 到 Length 依次输出。
void Linked_List_Print(Linked_List *list);

// ---- 元素存取 ----

// 获取位序为 ord 的元素（ord 从 1 开始），通过 elem 返回。
// elem 可以为 NULL，此时仅做越界检查而不获取值。
// 成功返回 true；list 无效或 ord 越界时打印错误并返回 false。
bool Linked_List_GetElem(Linked_List *list, int ord, int *elem);

// 更新位序为 ord 的元素（ord 从 1 开始），新值为 elem。
// 通过 oldElem 返回旧值（oldElem 可为 NULL，不获取旧值）。
// 成功返回 true；list 无效或 ord 越界时打印错误并返回 false。
bool Linked_List_PutElem(Linked_List *list, int ord, int elem, int *oldElem);

// ---- 查找 ----

// 按值查找元素，返回其位序（ord，>=1）。
// 从表头开始沿 Next 指针顺序遍历，找到第一个匹配的结点即返回其位序。
// 成功时返回位序；未找到、list 无效或表为空时返回 0（同时打印错误）。
int Linked_List_GetOrdOfElem(Linked_List *list, int elem);

// ---- 插入与删除（核心算法：通过操纵 Prev/Next 指针实现） ----

// 在指定位序 ord 插入新元素 elem（ord 从 1 开始，允许 ord = Length+1 即尾部追加）。
// 创建新结点并调整前后结点的 Prev/Next 指针。根据插入位置分为四种情况：
// 空表插入、表头插入、表尾插入、中间插入。
// 成功返回 true；list 无效、ord 越界或内存分配失败时打印错误并返回 false。
bool Linked_List_InsertAtOrd(Linked_List *list, int ord, int elem);

// 删除位序为 ord 的元素（ord 从 1 开始），通过 elem 返回被删除的值。
// elem 可以为 NULL（不获取被删除的值）。调整前后结点的 Prev/Next 指针后释放结点。
// 根据删除位置分为四种情况：删除唯一结点、删除表头、删除表尾、删除中间。
// 成功返回 true；list 无效或 ord 越界时打印错误并返回 false。
bool Linked_List_DeleteAtOrd(Linked_List *list, int ord, int *elem);

// ---- 栈操作（LIFO：栈顶 ≡ 表头，ord=1） ----
//
// 双向链表作为栈使用时，栈顶对应表头（第一个结点）。
// Push 在表头插入，Pop 从表头删除：最后进入的最先出栈（LIFO）。
//
// 注意：以下每个栈操作函数均独立实现了完整的算法逻辑和注释，
// 同时在注释中指出其等价于 Linked_List_InsertAtOrd / DeleteAtOrd
// 的哪种调用。这便于学生理解栈 ADT 如何在双向链表上实现。

// 入栈：在栈顶（表头）插入新元素 elem。
// 等价于 Linked_List_InsertAtOrd(list, 1, elem)。
bool Linked_Stack_Push(Linked_List *list, int elem);

// 出栈：从栈顶（表头）移除元素，通过 elem 返回其值。
// 等价于 Linked_List_DeleteAtOrd(list, 1, elem)。
bool Linked_Stack_Pop(Linked_List *list, int *elem);

// 查看栈顶元素（不出栈），通过 elem 返回其值。
// 等价于 Linked_List_GetElem(list, 1, elem)。
bool Linked_Stack_Peek(Linked_List *list, int *elem);

// ---- 队列操作（FIFO：队尾进、队首出） ----
//
// 双向链表作为队列使用时，队首对应表头（第一个结点），队尾对应表尾（最后一个结点）。
// Enqueue 在表尾追加，Dequeue 从表头删除：最先进入的最先出队（FIFO）。
//
// 注意：以下每个队列操作函数均独立实现了完整的算法逻辑和注释，
// 便于学生理解队列 ADT 如何在双向链表上实现。

// 入队：在队尾（表尾）追加新元素 elem。
// 等价于 Linked_List_InsertAtOrd(list, list->Length + 1, elem)。
bool Linked_Queue_Enqueue(Linked_List *list, int elem);

// 出队：从队首（表头）移除元素，通过 elem 返回其值。
// 注意：与 Linked_Stack_Pop 在实现上完全相同（都是从表头删除），
// 但此处独立实现以展示队列 FIFO 的 ADT 语义。
// 等价于 Linked_List_DeleteAtOrd(list, 1, elem)。
bool Linked_Queue_Dequeue(Linked_List *list, int *elem);

// 查看队首元素（不出队），通过 elem 返回其值。
// 等价于 Linked_List_GetElem(list, 1, elem)。
bool Linked_Queue_PeekFront(Linked_List *list, int *elem);
