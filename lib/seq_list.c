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
    for (int i = 0; i < seq->Length; i++)
    {
        int item = seq->Data[i];
        printf("  (%d) = %d,\n", i + 1, item); // 显示位序（从1开始）
    }
    printf("]\n");
}

SeqList_Result SeqList_GetItem(SeqList *seq, int index, int *item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_GetItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (index < 1 || seq->Length < index)
    {
        printf("<SeqList_GetItem> index 超出界限, index = %d, length = %d\n", index, seq->Length);
        return SeqList_Error_INDEX_OUT_OF_RANGE;
    }

    if (item != NULL)
    {
        *item = seq->Data[index - 1];
    }
    return SeqList_Success;
}

SeqList_Result SeqList_PutItem(SeqList *seq, int index, int item, int *oldItem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_PutItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (index < 1 || seq->Length < index)
    {
        printf("<SeqList_PutItem> index 超出界限, index = %d, length = %d\n", index, seq->Length);
        return SeqList_Error_INDEX_OUT_OF_RANGE;
    }

    if (oldItem != NULL)
    {
        *oldItem = seq->Data[index - 1];
    }

    seq->Data[index - 1] = item;

    return SeqList_Success;
}

int SeqList_IndexOfItem(SeqList *seq, int item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_IndexOfItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }

    if (seq->Length == 0)
    {
        return SeqList_Error_ITEM_NOT_FOUND;
    }

    for (int i = 0; i < seq->Length; i++)
    {
        if (seq->Data[i] == item)
        {
            return i + 1;
        }
    }

    return SeqList_Error_ITEM_NOT_FOUND;
}

SeqList_Result SeqList_InsertItem(SeqList *seq, int index, int item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_InsertItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (index < 1 || seq->Length + 1 < index)
    {
        printf("<SeqList_InsertItem> index 超出界限, index = %d, length = %d\n", index, seq->Length);
        return SeqList_Error_INDEX_OUT_OF_RANGE;
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

        // 复制元素
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

    // 向后移动元素，
    // index 是新元素要插入 seq 的位序（从1开始），
    // index - 1 是新元素要插入 data 的下标（从0开始）
    //
    // data[length - 1] -> data[length]
    // data[length - 2] -> data[length - 1]
    //                  ...
    // data[index]      -> data[index + 1]
    // data[index - 1]  -> data[index]
    for (int i = seq->Length; i >= index; i--)
    {
        seq->Data[i] = seq->Data[i - 1];
    }

    seq->Data[index - 1] = item;
    seq->Length++;

    return SeqList_Success;
}

SeqList_Result SeqList_DeleteItem(SeqList *seq, int index, int *item)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_DeleteItem> seq 为 NULL 或未正确初始化\n");
        return SeqList_Error_NULL_SEQ;
    }
    if (index < 1 || seq->Length < index)
    {
        printf("<SeqList_DeleteItem> index 超出界限, index = %d, length = %d\n", index, seq->Length);
        return SeqList_Error_INDEX_OUT_OF_RANGE;
    }

    if (item != NULL)
    {
        *item = seq->Data[index - 1];
    }

    // 向前移动元素，
    // index 是要删除元素在 seq 中的位序（从1开始），
    // index - 1 是要删除元素在 data 中的下标（从0开始）
    //
    // data[index]      -> data[index - 1]
    // data[index + 1]  -> data[index]
    //                  ...
    // data[length - 1] -> data[length - 2]
    for (int i = index; i <= seq->Length - 1; i++)
    {
        seq->Data[i - 1] = seq->Data[i];
    }

    seq->Length--;

    return SeqList_Success;
}
