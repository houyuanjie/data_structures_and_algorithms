#include "seq_list.h"

#include <stdio.h>
#include <stdlib.h>

SeqList_Result SeqList_Initialize(SeqList *seq)
{
    if (seq == NULL)
    {
        printf("<SeqList_Initialize> 要初始化的 seq 不能为 NULL\n");
        return SeqList_Error_NULL_SEQ;
    }

    seq->Data = (int *)malloc(sizeof(int) * SeqList_INITIAL_MAX_SIZE);
    if (seq->Data == NULL)
    {
        printf("<SeqList_Initialize> 空间分配失败\n");
        return SeqList_Error_ALLOC_FAILED;
    }

    seq->Length = 0;
    seq->MaxSize = SeqList_INITIAL_MAX_SIZE;

    return SeqList_Success;
}

SeqList_Result SeqList_Destroy(SeqList *seq)
{
    if (seq == NULL)
    {
        printf("<SeqList_Destroy> 要销毁的 seq 不能为 NULL\n");
        return SeqList_Error_NULL_SEQ;
    }

    if (seq->Data != NULL)
    {
        free(seq->Data);
    }

    seq->Data = NULL;
    seq->Length = 0;
    seq->MaxSize = 0;

    return SeqList_Success;
}

int SeqList_GetLength(SeqList *seq)
{
    if (seq == NULL)
    {
        printf("<SeqList_GetLength> seq 不能为 NULL\n");
        return SeqList_Error_NULL_SEQ;
    }
    return seq->Length;
}

bool SeqList_IsEmpty(SeqList *seq)
{
    return seq == NULL || seq->Data == NULL || seq->Length == 0;
}

void SeqList_Print(SeqList *seq)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("NULL\n");
        return;
    }

    if (seq->Length == 0)
    {
        printf("[]\n");
        return;
    }

    printf("[\n");
    // i 是数组下标（从0开始），显示时换算为位序 ord（从1开始）
    for (int i = 0; i < seq->Length; i++)
    {
        int ord = i + 1;
        int item = seq->Data[i];
        printf("  (%d) = %d,\n", ord, item);
    }
    printf("]\n");
}

SeqList_Result SeqList_GetItem(SeqList *seq, int ord, int *item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_GetItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (ord < 1 || seq->Length < ord)
    {
        printf("<SeqList_GetItem> ord 超出界限, ord = %d, length = %d\n", ord, seq->Length);
        return SeqList_Error_ORD_OUT_OF_RANGE;
    }

    // ord 是位序（从1开始），换算为数组下标 index（从0开始）
    int index = ord - 1;
    if (item != NULL)
    {
        *item = seq->Data[index];
    }
    return SeqList_Success;
}

SeqList_Result SeqList_PutItem(SeqList *seq, int ord, int item, int *oldItem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_PutItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (ord < 1 || seq->Length < ord)
    {
        printf("<SeqList_PutItem> ord 超出界限, ord = %d, length = %d\n", ord, seq->Length);
        return SeqList_Error_ORD_OUT_OF_RANGE;
    }

    // ord 是位序（从1开始），换算为数组下标 index（从0开始）
    int index = ord - 1;
    if (oldItem != NULL)
    {
        *oldItem = seq->Data[index];
    }

    seq->Data[index] = item;

    return SeqList_Success;
}

int SeqList_OrdOfItem(SeqList *seq, int item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_OrdOfItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }

    if (seq->Length == 0)
    {
        return SeqList_Error_ITEM_NOT_FOUND;
    }

    // i 是数组下标（从0开始），找到后返回对应的位序 ord（从1开始）
    for (int i = 0; i < seq->Length; i++)
    {
        if (seq->Data[i] == item)
        {
            int ord = i + 1;
            return ord;
        }
    }

    return SeqList_Error_ITEM_NOT_FOUND;
}

SeqList_Result SeqList_InsertItem(SeqList *seq, int ord, int item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_InsertItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (ord < 1 || seq->Length + 1 < ord)
    {
        printf("<SeqList_InsertItem> ord 超出界限, ord = %d, length = %d\n", ord, seq->Length);
        return SeqList_Error_ORD_OUT_OF_RANGE;
    }

    // 检查是否需要扩容
    if (seq->Length >= seq->MaxSize)
    {
        int newMaxSize = seq->MaxSize * 2;
        int *newData = (int *)malloc(sizeof(int) * newMaxSize);
        if (newData == NULL)
        {
            printf("<SeqList_InsertItem> seq 需要扩容时空间分配失败\n");
            return SeqList_Error_ALLOC_FAILED;
        }

        // 复制元素，i 是数组下标（从0开始）
        for (int i = 0; i < seq->Length; i++)
        {
            newData[i] = seq->Data[i];
        }

        // 释放旧的内存空间
        free(seq->Data);

        // 设置新的内存空间
        seq->MaxSize = newMaxSize;
        seq->Data = newData;
    }

    // 向后移动元素，为新元素腾出空间。
    // ord 是新元素要插入的位序（从1开始），
    // index = ord - 1 是新元素要插入的数组下标（从0开始）。
    //
    // 移动方向（数组下标 i 从 Length-1 递减到 index）：
    //   Data[Length-1] -> Data[Length]
    //   Data[Length-2] -> Data[Length-1]
    //                  ...
    //   Data[index]    -> Data[index+1]
    int index = ord - 1;
    for (int i = seq->Length - 1; i >= index; i--)
    {
        seq->Data[i + 1] = seq->Data[i];
    }

    seq->Data[index] = item;
    seq->Length++;

    return SeqList_Success;
}

SeqList_Result SeqList_DeleteItem(SeqList *seq, int ord, int *item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_DeleteItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (ord < 1 || seq->Length < ord)
    {
        printf("<SeqList_DeleteItem> ord 超出界限, ord = %d, length = %d\n", ord, seq->Length);
        return SeqList_Error_ORD_OUT_OF_RANGE;
    }

    // ord 是位序（从1开始），换算为数组下标 index（从0开始）
    int index = ord - 1;
    if (item != NULL)
    {
        *item = seq->Data[index];
    }

    // 向前移动元素，覆盖被删除的元素。
    // index 是要删除元素的数组下标（从0开始）。
    //
    // 移动方向（数组下标 i 从 index+1 递增到 Length-1）：
    //   Data[index+1] -> Data[index]
    //   Data[index+2] -> Data[index+1]
    //                  ...
    //   Data[Length-1] -> Data[Length-2]
    for (int i = index + 1; i <= seq->Length - 1; i++)
    {
        seq->Data[i - 1] = seq->Data[i];
    }

    seq->Length--;

    return SeqList_Success;
}
