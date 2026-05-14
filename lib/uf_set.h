#pragma once

// ==========================================================================
// 并查集 (Union-Find Set / Disjoint Set) —— 双亲表示法
//
// 用一个数组 Data[0..Size-1] 表示树形结构（森林）：
//   - Data[i] < 0  → i 是根结点，|Data[i]| 表示集合大小（目前固定为 1）
//   - Data[i] >= 0 → i 的父结点为 Data[i]
//
// 支持两种基本操作：
//   Find  —— 查找元素所属集合的根
//   Union —— 合并两个集合（将一个根指向另一个根）
//
// 元素用其在数组中的下标 (0..Size-1) 标识，因而 Size 决定了可容纳的
// 最大元素个数。
// ==========================================================================

typedef struct UFSet
{
    int Size;  // 集合容量（最大元素个数）
    int *Data; // 指向双亲表示数组的指针
} UFSet;

// ---- 创建与销毁 ----

// 初始化并查集，分配 size 个元素的空间。
// 每个元素初始时自成一个集合（Data[i] = -1，自己就是根）。
// set 为 NULL、size <= 0 或内存分配失败时无操作。
void UFSet_Initialize(UFSet *set, int size);

// 销毁并查集，释放 Data 内存并将 Size 置 0、Data 置 NULL。
// set 为 NULL 或 Data 已为 NULL 时无操作。
void UFSet_Destroy(UFSet *set);

// ---- 调试输出 ----

// 打印并查集的内部状态：容量和每个元素的父指针 / 根标记。
void UFSet_Print(UFSet *set);

// ---- 核心操作 ----

// 查找元素 elem 所在集合的根（elem 为数组下标 0..Size-1）。
// 沿着父指针链向上追溯，直到遇到 Data[i] < 0 的根结点，返回其下标。
int UFSet_Find(UFSet *set, int elem);

// 合并两个集合，将 root2 的根指向 root1。
// root1 和 root2 必须是通过 UFSet_Find 得到的根下标。
// 若 root1 == root2（已在同一集合中）则无操作。
void UFSet_Union(UFSet *set, int root1, int root2);
