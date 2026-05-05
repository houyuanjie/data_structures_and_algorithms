#pragma once

#include <stdbool.h>

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

// 初始时分配的内存空间（可容纳的元素个数）
#define SeqList_INITIAL_MAX_SIZE 16

// 动态分配的顺序表
typedef struct SeqList
{
    int *Data;      // 指向动态分配的数组空间
    int Length;     // 当前元素个数（表长）
    int MaxSize;    // 当前已分配的最大容量
} SeqList;

/* ---- 创建与销毁 ---- */

// 初始化顺序表。分配初始容量的内存，Length = 0，MaxSize = 初始容量。
// 成功返回 true；seq 为 NULL 或内存分配失败时打印错误并返回 false。
bool SeqList_Initialize(SeqList *seq);

// 销毁顺序表。释放 Data 指向的内存，并将三个字段重置为 0/NULL。
// 成功返回 true；seq 为 NULL 时打印错误并返回 false。
bool SeqList_Destroy(SeqList *seq);

/* ---- 状态查询 ---- */

// 获取表长（当前元素个数）。
// 返回 Length（>=0）；seq 为 NULL 时打印错误并返回 0。
int SeqList_GetLength(SeqList *seq);

// 判断是否为空表。
// seq 为 NULL、未初始化或 Length == 0 时均返回 true。
bool SeqList_IsEmpty(SeqList *seq);

// 打印表中的所有元素。
// 按位序 (ord) 从 1 到 Length 依次输出。
void SeqList_Print(SeqList *seq);

/* ---- 元素存取 ---- */

// 获取位序为 ord 的元素（ord 从 1 开始），通过 elem 返回。
// elem 可以为 NULL，此时仅做越界检查而不获取值。
// 成功返回 true；seq 无效或 ord 越界时打印错误并返回 false。
bool SeqList_GetElem(SeqList *seq, int ord, int *elem);

// 更新位序为 ord 的元素（ord 从 1 开始），新值为 elem。
// 通过 oldElem 返回旧值（oldElem 可为 NULL，不获取旧值）。
// 成功返回 true；seq 无效或 ord 越界时打印错误并返回 false。
bool SeqList_PutElem(SeqList *seq, int ord, int elem, int *oldElem);

/* ---- 查找 ---- */

// 按值查找元素，返回其位序（ord，>=1）。
// 成功时返回位序；未找到、seq 无效或表为空时返回 0（同时打印错误）。
int SeqList_GetOrdOfElem(SeqList *seq, int elem);

/* ---- 插入与删除（核心算法） ---- */

// 在指定位序 ord 插入新元素 elem（ord 从 1 开始，允许 ord = Length+1 即尾部追加）。
// 插入位置及其后的元素依次后移一格；表满时自动扩容（容量 ×2）。
// 成功返回 true；seq 无效、ord 越界或扩容失败时打印错误并返回 false。
bool SeqList_InsertElem(SeqList *seq, int ord, int elem);

// 删除位序为 ord 的元素（ord 从 1 开始），通过 elem 返回被删除的值。
// elem 可以为 NULL（不获取被删除的值）；删除位置之后的元素依次前移一格。
// 成功返回 true；seq 无效或 ord 越界时打印错误并返回 false。
bool SeqList_DeleteElem(SeqList *seq, int ord, int *elem);
