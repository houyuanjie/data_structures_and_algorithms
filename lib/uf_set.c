#include "uf_set.h"

#include <stdio.h>
#include <stdlib.h>

// ==========================================================================
// 并查集 (UFSet) 实现 —— 双亲表示法
//
// 内部约定：
//   - elem / root / i / j 均表示数组 Data 的下标（0-based）
//   - Data[i] < 0  → i 是根（值为 -1，即集合大小为 1）
//   - Data[i] >= 0 → i 的父结点下标
// ==========================================================================

// ==========================================================================
// 创建与销毁
// ==========================================================================

void UFSet_Initialize(UFSet *set, int size)
{
    if (set == NULL)
    {
        return;
    }
    if (size <= 0)
    {
        return;
    }

    int *newData = (int *)malloc(sizeof(int) * size);
    if (newData == NULL)
    {
        return;
    }

    // 每个元素初始化为根结点（-1 表示自成一个集合）
    for (int i = 0; i < size; i++)
    {
        newData[i] = -1;
    }

    set->Size = size;
    set->Data = newData;
}

void UFSet_Destroy(UFSet *set)
{
    if (set == NULL || set->Data == NULL)
    {
        return;
    }

    set->Size = 0;
    free(set->Data);
    set->Data = NULL;
}

// ==========================================================================
// 调试输出
// ==========================================================================

void UFSet_Print(UFSet *set)
{
    if (set == NULL || set->Data == NULL)
    {
        printf("(null)\n");
        return;
    }

    printf("UFSet: Size = %d\n", set->Size);

    // 逐元素打印：下标、父指针 / 根标记
    for (int i = 0; i < set->Size; i++)
    {
        if (set->Data[i] < 0)
        {
            // i 是根结点
            printf("  [%d] = %d (root)\n", i, set->Data[i]);
        }
        else
        {
            // i 的父结点为 Data[i]
            printf("  [%d] = %d\n", i, set->Data[i]);
        }
    }
}

// ==========================================================================
// 查找 —— 沿父指针链追溯根结点
//
// 从元素 elem 出发，不断向上访问父结点，直到遇到 Data[i] < 0 的根。
//
// 时间复杂度：最坏 O(n)（树退化为链表时）
// ==========================================================================

int UFSet_Find(UFSet *set, int elem)
{
    // 从 elem 开始，沿父指针链向上追溯
    int root = elem;
    while (set->Data[root] >= 0)
    {
        root = set->Data[root];
    }
    return root;
}

// ==========================================================================
// 合并 —— 将一个根指向另一个根
//
// 将 root2 的父指针指向 root1，使两棵树合并。
//
// 注意：调用者应确保 root1、root2 是 UFSet_Find 返回的根下标，
// 且 set 已经正确初始化。
//
// 时间复杂度：O(1)
// ==========================================================================

void UFSet_Union(UFSet *set, int root1, int root2)
{
    // 已在同一集合中，无需合并
    if (root1 == root2)
    {
        return;
    }

    // 将 root2 的父指针指向 root1
    set->Data[root2] = root1;
}
