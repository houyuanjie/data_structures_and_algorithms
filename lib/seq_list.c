#include "seq_list.h"

#include <stdio.h>
#include <stdlib.h>

/* ==========================================================================
 * 顺序表 (SeqList) 实现
 *
 * 所有对外函数在遇到错误时均会打印带函数名的诊断消息，然后返回 false 或 0，
 * 这使得调用方可以简单地用 if (!Func(...)) 判断失败，无需管理错误码。
 *
 * 内部约定：
 *   - ord 一律表示位序（1-based），来自外部调用者
 *   - index / i / j / k 一律表示数组下标（0-based），仅在实现内部使用
 *   - 两者通过 index = ord - 1 和 ord = index + 1 相互转换
 * ========================================================================== */

/* ==========================================================================
 * 创建与销毁
 * ========================================================================== */

bool SeqList_Initialize(SeqList *seq)
{
    if (seq == NULL)
    {
        printf("<SeqList_Initialize> 参数 seq 为 NULL，无法初始化\n");
        return false;
    }

    seq->Data = (int *)malloc(sizeof(int) * SeqList_INITIAL_MAX_SIZE);
    if (seq->Data == NULL)
    {
        printf("<SeqList_Initialize> 内存分配失败（请求 %d 个元素的空间）\n",
               SeqList_INITIAL_MAX_SIZE);
        return false;
    }

    seq->Length = 0;
    seq->MaxSize = SeqList_INITIAL_MAX_SIZE;

    return true;
}

bool SeqList_Destroy(SeqList *seq)
{
    if (seq == NULL)
    {
        printf("<SeqList_Destroy> 参数 seq 为 NULL，无需销毁\n");
        return false;
    }

    if (seq->Data != NULL)
    {
        free(seq->Data);
    }

    seq->Data = NULL;
    seq->Length = 0;
    seq->MaxSize = 0;

    return true;
}

/* ==========================================================================
 * 状态查询
 * ========================================================================== */

int SeqList_GetLength(SeqList *seq)
{
    if (seq == NULL)
    {
        printf("<SeqList_GetLength> 参数 seq 为 NULL，返回 0\n");
        return 0;
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
        printf("(null)\n");
        return;
    }

    if (seq->Length == 0)
    {
        printf("[]\n");
        return;
    }

    printf("[\n");
    // i 为数组下标（0-based），显示时换算为位序 ord（1-based）
    for (int i = 0; i < seq->Length; i++)
    {
        int ord = i + 1;
        printf("  [%2d] = %d,\n", ord, seq->Data[i]);
    }
    printf("]\n");
}

/* ==========================================================================
 * 元素存取
 * ========================================================================== */

bool SeqList_GetElem(SeqList *seq, int ord, int *elem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_GetElem> seq 为 NULL 或未初始化\n");
        return false;
    }
    if (ord < 1 || seq->Length < ord)
    {
        printf("<SeqList_GetElem> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
               ord, seq->Length, seq->Length);
        return false;
    }

    // ord 是位序（1-based），换算为数组下标 index（0-based）
    int index = ord - 1;
    if (elem != NULL)
    {
        *elem = seq->Data[index];
    }
    return true;
}

bool SeqList_PutElem(SeqList *seq, int ord, int elem, int *oldElem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_PutElem> seq 为 NULL 或未初始化\n");
        return false;
    }
    if (ord < 1 || seq->Length < ord)
    {
        printf("<SeqList_PutElem> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
               ord, seq->Length, seq->Length);
        return false;
    }

    // ord 是位序（1-based），换算为数组下标 index（0-based）
    int index = ord - 1;
    if (oldElem != NULL)
    {
        *oldElem = seq->Data[index];
    }

    seq->Data[index] = elem;

    return true;
}

/* ==========================================================================
 * 查找 —— 顺序查找算法
 *
 * 从第一个元素开始，逐一与目标值比较，找到即返回其位序。
 * 时间复杂度 O(n)，空间复杂度 O(1)。
 * ========================================================================== */

int SeqList_GetOrdOfElem(SeqList *seq, int elem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_GetOrdOfElem> seq 为 NULL 或未初始化\n");
        return 0;
    }

    if (seq->Length == 0)
    {
        printf("<SeqList_GetOrdOfElem> 表为空，元素 %d 不存在\n", elem);
        return 0;
    }

    // 顺序扫描：i 为数组下标（0-based），找到后返回位序 ord（1-based）
    for (int i = 0; i < seq->Length; i++)
    {
        if (seq->Data[i] == elem)
        {
            return i + 1; // 下标 → 位序
        }
    }

    printf("<SeqList_GetOrdOfElem> 元素 %d 未找到\n", elem);
    return 0;
}

/* ==========================================================================
 * 插入 —— 核心算法
 *
 * 在指定位序 ord 处插入新元素 elem。算法分三步：
 *
 * 第 1 步：越界检查
 *   有效插入位序范围为 1 .. Length+1，其中 Length+1 表示尾部追加。
 *
 * 第 2 步：必要时扩容
 *   当 Length == MaxSize 时，表已满，无法再插入。此时分配一块容量翻倍的
 *   新数组，将旧数组的元素逐一下标复制过去，释放旧数组，再更新 Data 指针
 *   和 MaxSize。
 *
 * 第 3 步：后移腾位 + 写入新值
 *   从最后一个元素开始，到插入位置为止，每个元素向后移一格：
 *
 *       移动前:  A[0]  A[1]  ...  A[index]  ...  A[Length-1]
 *       移动后:  A[0]  A[1]  ...   (空)    ...  A[Length-1]  A[Length]
 *                                           ↑
 *                                      插入 elem
 *
 *   具体做法：令 i 从 Length-1 递减到 index，每次执行 Data[i+1] = Data[i]。
 *   当循环结束时，Data[index] 已被复制到 Data[index+1]，该位置空出。
 *   最后写入 Data[index] = elem，Length 加 1。
 *
 * 时间复杂度 O(n)（移动元素），空间复杂度 O(1)（不计扩容时的临时数组）。
 * ========================================================================== */

bool SeqList_InsertElem(SeqList *seq, int ord, int elem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_InsertElem> seq 为 NULL 或未初始化\n");
        return false;
    }
    if (ord < 1 || seq->Length + 1 < ord)
    {
        printf("<SeqList_InsertElem> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
               ord, seq->Length, seq->Length + 1);
        return false;
    }

    /* ---- 第 2 步：必要时扩容 ---- */
    if (seq->Length >= seq->MaxSize)
    {
        int newMaxSize = seq->MaxSize * 2;
        int *newData = (int *)malloc(sizeof(int) * newMaxSize);
        if (newData == NULL)
        {
            printf("<SeqList_InsertElem> 扩容失败（无法分配 %d 个元素的空间）\n",
                   newMaxSize);
            return false;
        }

        // 将旧数组的元素逐一下标复制到新数组
        for (int i = 0; i < seq->Length; i++)
        {
            newData[i] = seq->Data[i];
        }

        free(seq->Data);
        seq->Data = newData;
        seq->MaxSize = newMaxSize;
    }

    /* ---- 第 3 步：后移腾位 + 写入新值 ---- */

    // ord → 数组下标
    int index = ord - 1;

    // 从最后一个元素开始，逐个向后移动一格，直到腾出 index 位置
    //
    // 示例（Length=5，在 ord=2 即 index=1 处插入）：
    //   i=4: Data[5] = Data[4]    [0] [1] [2] [3] [4] [5]
    //   i=3: Data[4] = Data[3]     A   B   C   D   E   ?
    //   i=2: Data[3] = Data[2]     ↓   ↓   ↓   ↓       ↓
    //   i=1: Data[2] = Data[1]     A   B   B   C   D   E
    //        Data[1] = elem        A  NEW  B   C   D   E
    for (int i = seq->Length - 1; i >= index; i--)
    {
        seq->Data[i + 1] = seq->Data[i];
    }

    seq->Data[index] = elem;
    seq->Length++;

    return true;
}

/* ==========================================================================
 * 删除 —— 核心算法
 *
 * 删除位序为 ord 的元素。算法分两步：
 *
 * 第 1 步：越界检查
 *   有效删除位序范围为 1 .. Length。
 *
 * 第 2 步：取出被删元素 + 前移覆盖
 *   被删元素在 Data[index]（index = ord - 1）。先用 elem 指针传出该值，
 *   然后将其后的所有元素依次向前移动一格：
 *
 *       移动前:  A[0]  ...  A[index]  A[index+1]  ...  A[Length-1]
 *       移动后:  A[0]  ...  A[index+1]  ...  A[Length-1]
 *
 *   具体做法：令 i 从 index+1 递增到 Length-1，每次执行 Data[i-1] = Data[i]。
 *   循环结束后，原来 index 位置的值被覆盖，Length 减 1。
 *
 * 时间复杂度 O(n)，空间复杂度 O(1)。
 * ========================================================================== */

bool SeqList_DeleteElem(SeqList *seq, int ord, int *elem)
{
    if (seq == NULL || seq->Data == NULL)
    {
        printf("<SeqList_DeleteElem> seq 为 NULL 或未初始化\n");
        return false;
    }
    if (ord < 1 || seq->Length < ord)
    {
        printf("<SeqList_DeleteElem> 位序 ord = %d 越界（表长 = %d，有效位序 1..%d）\n",
               ord, seq->Length, seq->Length);
        return false;
    }

    // ord → 数组下标
    int index = ord - 1;

    // 传出被删除的元素
    if (elem != NULL)
    {
        *elem = seq->Data[index];
    }

    // 将后继元素逐个向前移动一格，覆盖被删位置
    //
    // 示例（Length=5，删除 ord=2 即 index=1 处的元素 B）：
    //   i=2: Data[1] = Data[2]    [0] [1] [2] [3] [4]
    //   i=3: Data[2] = Data[3]     A   B   C   D   E
    //   i=4: Data[3] = Data[4]         ↓   ↓   ↓
    //                               A   C   D   E   E
    //   Length-- 后逻辑上变为:      A   C   D   E
    for (int i = index + 1; i <= seq->Length - 1; i++)
    {
        seq->Data[i - 1] = seq->Data[i];
    }

    seq->Length--;

    return true;
}
