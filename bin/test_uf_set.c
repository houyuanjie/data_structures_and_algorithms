#include "uf_set.h"

#include <stdio.h>

// 测试宏：检查条件并打印结果
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

int test_initialize_and_destroy()
{
    printf("\n=== 初始化与销毁测试 ===\n");

    UFSet set;

    // 正常初始化
    UFSet_Initialize(&set, 8);
    TEST("初始化后 Size = 8", set.Size == 8);
    TEST("初始化后 Data != NULL", set.Data != NULL);

    // 每个元素初始时应该是自己的根
    int all_roots = 1;
    for (int i = 0; i < 8; i++)
    {
        if (set.Data[i] != -1)
        {
            all_roots = 0;
            break;
        }
    }
    TEST("每个元素初始为根 (Data[i] = -1)", all_roots);

    // 销毁
    UFSet_Destroy(&set);
    TEST("销毁后 Size = 0", set.Size == 0);
    TEST("销毁后 Data = NULL", set.Data == NULL);

    // 边界：NULL 初始化（不应崩溃）
    UFSet_Initialize(NULL, 8);
    TEST("NULL 初始化 (不崩溃)", 1);

    // 边界：size <= 0
    UFSet set2 = {0};
    UFSet_Initialize(&set2, 0);
    TEST("size=0 初始化 (不分配内存)", set2.Data == NULL);

    UFSet_Initialize(&set2, -1);
    TEST("size=-1 初始化 (不分配内存)", set2.Data == NULL);

    // 边界：NULL 销毁
    UFSet_Destroy(NULL);
    TEST("NULL 销毁 (不崩溃)", 1);

    return 0;
}

int test_find()
{
    printf("\n=== 查找测试 ===\n");

    UFSet set;
    UFSet_Initialize(&set, 8);

    // 初始时每个元素自成一个集合，Find 应返回自身
    TEST("Find(0) = 0", UFSet_Find(&set, 0) == 0);
    TEST("Find(3) = 3", UFSet_Find(&set, 3) == 3);
    TEST("Find(7) = 7", UFSet_Find(&set, 7) == 7);

    // 手工构建一条链：0 ← 1 ← 2 (0 为根)
    set.Data[1] = 0; // 1 的父结点是 0
    set.Data[2] = 1; // 2 的父结点是 1

    TEST("Find(2) 沿链追溯 = 0", UFSet_Find(&set, 2) == 0);
    TEST("Find(1) = 0", UFSet_Find(&set, 1) == 0);
    TEST("Find(0) 仍是根", UFSet_Find(&set, 0) == 0);
    // 未连接的结点不受影响
    TEST("Find(3) 仍是 3", UFSet_Find(&set, 3) == 3);

    UFSet_Destroy(&set);
    return 0;
}

int test_union()
{
    printf("\n=== 合并测试 ===\n");

    UFSet set;
    UFSet_Initialize(&set, 8);

    // 合并元素 0 和 1：将 root 1 指向 root 0
    UFSet_Union(&set, 0, 1);
    // 现在 1 的父结点应为 0
    TEST("Union(0,1): Data[1] = 0", set.Data[1] == 0);
    // Find(1) 应该找到 0
    TEST("Union(0,1): Find(1) = 0", UFSet_Find(&set, 1) == 0);
    // Find(0) 仍是 0
    TEST("Union(0,1): Find(0) = 0", UFSet_Find(&set, 0) == 0);

    // 合并元素 2 和 3
    UFSet_Union(&set, 2, 3);
    TEST("Union(2,3): Data[3] = 2", set.Data[3] == 2);
    TEST("Union(2,3): Find(3) = 2", UFSet_Find(&set, 3) == 2);

    // 合并两个集合：(0,1) 和 (2,3)
    int root0 = UFSet_Find(&set, 0); // = 0
    int root2 = UFSet_Find(&set, 2); // = 2
    UFSet_Union(&set, root0, root2); // 将 root 2 指向 root 0
    TEST("合并两个集合后 Find(3) = 0", UFSet_Find(&set, 3) == 0);
    TEST("合并两个集合后 Find(1) = 0", UFSet_Find(&set, 1) == 0);

    // 已合并的集合再次合并应无影响
    int root0_again = UFSet_Find(&set, 0); // = 0
    int root3 = UFSet_Find(&set, 3);       // = 0 (same set)
    TEST("同一集合 root 相同", root0_again == root3);
    // Union 相同 root 不会改变 Data
    int old_data_1 = set.Data[1];
    UFSet_Union(&set, root0_again, root3);
    TEST("Union 相同 root 无影响", set.Data[1] == old_data_1);

    UFSet_Destroy(&set);
    return 0;
}

int test_long_chain()
{
    printf("\n=== 长链测试 ===\n");

    UFSet set;
    UFSet_Initialize(&set, 6);

    // 构建链: 0 ← 1 ← 2 ← 3 ← 4 ← 5
    // 逐步 Union
    UFSet_Union(&set, 0, 1); // 1→0
    UFSet_Union(&set, 0, 2); // 2→0 (注意：2 的根是 2，合并后指向 0)
    UFSet_Union(&set, 0, 3); // 3→0
    UFSet_Union(&set, 0, 4); // 4→0
    UFSet_Union(&set, 0, 5); // 5→0

    // 所有元素的根都应该是 0
    int all_root_0 = 1;
    for (int i = 0; i < 6; i++)
    {
        if (UFSet_Find(&set, i) != 0)
        {
            all_root_0 = 0;
            break;
        }
    }
    TEST("所有元素归入同一集合 (根 = 0)", all_root_0);

    UFSet_Destroy(&set);
    return 0;
}

int test_multiple_sets()
{
    printf("\n=== 多集合测试 ===\n");

    UFSet set;
    UFSet_Initialize(&set, 10);

    // 构建三个集合：
    //   集合 A: {0, 1, 2}  根=0
    //   集合 B: {3, 4}     根=3
    //   集合 C: {5, 6, 7}  根=5
    //   孤立:  {8}, {9}
    UFSet_Union(&set, 0, 1); // 1→0
    UFSet_Union(&set, 0, 2); // 2→0

    UFSet_Union(&set, 3, 4); // 4→3

    UFSet_Union(&set, 5, 6); // 6→5
    UFSet_Union(&set, 5, 7); // 7→5

    // 验证各集合的根
    TEST("集合 A: Find(0)=0, Find(1)=0, Find(2)=0",
         UFSet_Find(&set, 0) == 0 && UFSet_Find(&set, 1) == 0 &&
             UFSet_Find(&set, 2) == 0);

    TEST("集合 B: Find(3)=3, Find(4)=3",
         UFSet_Find(&set, 3) == 3 && UFSet_Find(&set, 4) == 3);

    TEST("集合 C: Find(5)=5, Find(6)=5, Find(7)=5",
         UFSet_Find(&set, 5) == 5 && UFSet_Find(&set, 6) == 5 &&
             UFSet_Find(&set, 7) == 5);

    TEST("孤立元素: Find(8)=8, Find(9)=9",
         UFSet_Find(&set, 8) == 8 && UFSet_Find(&set, 9) == 9);

    // 合并集合 A 和 B
    int rootA = UFSet_Find(&set, 0); // = 0
    int rootB = UFSet_Find(&set, 3); // = 3
    UFSet_Union(&set, rootA, rootB);

    TEST("合并 A+B 后: Find(4) = 0", UFSet_Find(&set, 4) == 0);
    TEST("合并 A+B 后: Find(3) = 0", UFSet_Find(&set, 3) == 0);

    // 合并 (A+B) 和 C
    int rootAB = UFSet_Find(&set, 0); // = 0
    int rootC = UFSet_Find(&set, 5);  // = 5
    UFSet_Union(&set, rootAB, rootC);

    TEST("合并 (A+B)+C 后: Find(7) = 0", UFSet_Find(&set, 7) == 0);

    UFSet_Destroy(&set);
    return 0;
}

int test_print()
{
    printf("\n=== 打印测试 ===\n");

    UFSet set;
    UFSet_Initialize(&set, 5);

    // 构建: 0←1←2  3←4
    UFSet_Union(&set, 0, 1);
    UFSet_Union(&set, 0, 2);
    UFSet_Union(&set, 3, 4);

    printf("  并查集内部状态:\n");
    UFSet_Print(&set);

    // 打印 null 集
    printf("  空集:\n");
    UFSet_Print(NULL);

    UFSet_Destroy(&set);
    return 0;
}

int main()
{
    printf("=======================================\n");
    printf("      并查集 (UFSet) 测试程序          \n");
    printf("=======================================\n");

    int result = 0;

    result |= test_initialize_and_destroy();
    result |= test_find();
    result |= test_union();
    result |= test_long_chain();
    result |= test_multiple_sets();
    result |= test_print();

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
