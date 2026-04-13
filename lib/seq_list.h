#pragma once

#include <stdbool.h>

// 初始时分配的内存空间
#define SeqList_INITIAL_MAX_SIZE 16

// 操作结果枚举
typedef enum SeqList_Result
{
    // 操作成功
    SeqList_Success = 0,
    // 内存分配失败
    SeqList_Error_ALLOC_FAILED = -1,
    // seq 为 NULL 或未正确初始化
    SeqList_Error_NULL_SEQ = -2,
    // 元素未找到
    SeqList_Error_ITEM_NOT_FOUND = -3,
    // 索引越界
    SeqList_Error_INDEX_OUT_OF_RANGE = -4,
} SeqList_Result;

// 动态分配的顺序表
typedef struct SeqList
{
    int *Data;
    int Length;
    int MaxSize;
} SeqList;

// 初始化顺序表，
// 成功时返回 SeqList_Success，
// seq 为 NULL 时返回 SeqList_Error_NULL_SEQ，
// 内存分配失败时返回 SeqList_Error_ALLOC_FAILED
SeqList_Result SeqList_Initialize(SeqList *seq);

// 销毁顺序表，释放 Data 指向的内存并将字段重置为 0/NULL，
// 成功时返回 SeqList_Success，
// seq 为 NULL 时返回 SeqList_Error_NULL_SEQ
SeqList_Result SeqList_Destroy(SeqList *seq);

// 获取表长，
// 成功时返回当前长度（>=0），
// seq 为 NULL 时返回 SeqList_Error_NULL_SEQ（负值）
int SeqList_GetLength(SeqList *seq);

// 判断是否为空表，
// seq 为 NULL、未初始化或长度为 0 时均返回 true
bool SeqList_IsEmpty(SeqList *seq);

// 打印表中的元素，无返回值
void SeqList_Print(SeqList *seq);

// 获取位序为 index 的元素（index 从 1 开始），
// 通过 item 返回该元素（item 可为 NULL，仅检查不获取），
// 成功时返回 SeqList_Success，
// seq 无效时返回 SeqList_Error_NULL_SEQ，
// index 越界时返回 SeqList_Error_INDEX_OUT_OF_RANGE
SeqList_Result SeqList_GetItem(SeqList *seq, int index, int *item);

// 更新位序为 index 的元素（index 从 1 开始），新值为 item，
// 通过 oldItem 返回旧值（oldItem 可为 NULL，不获取旧值），
// 成功时返回 SeqList_Success，
// seq 无效时返回 SeqList_Error_NULL_SEQ，
// index 越界时返回 SeqList_Error_INDEX_OUT_OF_RANGE
SeqList_Result SeqList_PutItem(SeqList *seq, int index, int item, int *oldItem);

// 获取值为 item 的元素的位序（index 从 1 开始），
// 成功时返回位序（>=1），
// 未找到时返回 SeqList_Error_ITEM_NOT_FOUND，
// seq 无效时返回 SeqList_Error_NULL_SEQ
int SeqList_IndexOfItem(SeqList *seq, int item);

// 在指定位置插入新元素 item（index 从 1 开始，可在末尾+1处插入），
// 后继元素依次后移，必要时自动扩容，
// 成功时返回 SeqList_Success，
// seq 无效时返回 SeqList_Error_NULL_SEQ，
// index 越界时返回 SeqList_Error_INDEX_OUT_OF_RANGE，
// 扩容内存分配失败时返回 SeqList_Error_ALLOC_FAILED
SeqList_Result SeqList_InsertItem(SeqList *seq, int index, int item);

// 删除位序为 index 的元素（index 从 1 开始），
// 后继元素依次前移，
// 通过 item 返回被删除的元素（item 可为 NULL，不获取），
// 成功时返回 SeqList_Success，
// seq 无效时返回 SeqList_Error_NULL_SEQ，
// index 越界时返回 SeqList_Error_INDEX_OUT_OF_RANGE
SeqList_Result SeqList_DeleteItem(SeqList *seq, int index, int *item);
