#include <stdio.h>
#include <assert.h>
#include "seq_list.h"

// 测试宏：检查布尔表达式并打印结果
#define TEST(name, expr)                             \
    do                                               \
    {                                                \
        printf("  [测试] %-40s ", name);             \
        if (expr)                                    \
        {                                            \
            printf("✓ 通过\n");                      \
        }                                            \
        else                                         \
        {                                            \
            printf("✗ 失败 (第 %d 行)\n", __LINE__); \
            return 1;                                \
        }                                            \
    } while (0)

int test_basic_operations()
{
    printf("\n=== 基本操作测试 ===\n");

    SeqList seq;
    int elem;

    // 初始化
    TEST("初始化顺序表", SeqList_Initialize(&seq));
    TEST("初始长度为 0", SeqList_GetLength(&seq) == 0);
    TEST("初始为空表", SeqList_IsEmpty(&seq));

    // 插入测试
    TEST("在位序 1 插入 10", SeqList_InsertElem(&seq, 1, 10));
    TEST("在位序 2 插入 20", SeqList_InsertElem(&seq, 2, 20));
    TEST("在位序 1 插入 5 (头部)", SeqList_InsertElem(&seq, 1, 5));
    TEST("当前长度为 3", SeqList_GetLength(&seq) == 3);

    // 打印当前状态
    printf("  当前顺序表内容:\n");
    SeqList_Print(&seq);

    // 获取测试
    TEST("获取位序 1 的元素", SeqList_GetElem(&seq, 1, &elem));
    TEST("位序 1 的元素是 5", elem == 5);

    TEST("获取位序 2 的元素", SeqList_GetElem(&seq, 2, &elem));
    TEST("位序 2 的元素是 10", elem == 10);

    TEST("获取位序 3 的元素", SeqList_GetElem(&seq, 3, &elem));
    TEST("位序 3 的元素是 20", elem == 20);

    // 修改测试
    int oldElem;
    TEST("修改位序 2 的元素为 15", SeqList_PutElem(&seq, 2, 15, &oldElem));
    TEST("旧值为 10", oldElem == 10);
    TEST("获取修改后的值", SeqList_GetElem(&seq, 2, &elem));
    TEST("新值为 15", elem == 15);

    // 查找测试
    TEST("查找 5 的位序为 1", SeqList_GetOrdOfElem(&seq, 5) == 1);
    TEST("查找 15 的位序为 2", SeqList_GetOrdOfElem(&seq, 15) == 2);
    TEST("查找 20 的位序为 3", SeqList_GetOrdOfElem(&seq, 20) == 3);
    TEST("查找不存在的元素返回 0", SeqList_GetOrdOfElem(&seq, 999) == 0);

    // 删除测试
    TEST("删除位序 2 的元素", SeqList_DeleteElem(&seq, 2, &elem));
    TEST("删除的元素是 15", elem == 15);
    TEST("删除后长度为 2", SeqList_GetLength(&seq) == 2);

    TEST("获取新的位序 2", SeqList_GetElem(&seq, 2, &elem));
    TEST("现在位序 2 的元素是 20", elem == 20);

    // 销毁
    TEST("销毁顺序表", SeqList_Destroy(&seq));
    TEST("销毁后为空表", SeqList_IsEmpty(&seq));

    return 0;
}

int test_boundary_conditions()
{
    printf("\n=== 边界条件测试 ===\n");

    SeqList seq;
    int elem;

    SeqList_Initialize(&seq);

    // 越界插入测试
    TEST("在位序 0 插入 (越界，应失败)", !SeqList_InsertElem(&seq, 0, 1));
    TEST("在位序 2 插入空表 (越界，应失败)", !SeqList_InsertElem(&seq, 2, 1));

    // 插入一个元素后测试越界
    SeqList_InsertElem(&seq, 1, 100);
    TEST("在位序 3 插入 (越界，应失败)", !SeqList_InsertElem(&seq, 3, 1));

    // 越界获取/修改/删除
    TEST("获取位序 0 (越界，应失败)", !SeqList_GetElem(&seq, 0, &elem));
    TEST("获取位序 2 (越界，应失败)", !SeqList_GetElem(&seq, 2, &elem));
    TEST("修改位序 0 (越界，应失败)", !SeqList_PutElem(&seq, 0, 1, NULL));
    TEST("删除位序 0 (越界，应失败)", !SeqList_DeleteElem(&seq, 0, &elem));

    // NULL 指针测试
    TEST("NULL 初始化 (应失败)", !SeqList_Initialize(NULL));
    TEST("NULL 销毁 (应失败)", !SeqList_Destroy(NULL));
    TEST("NULL 获取长度返回 0", SeqList_GetLength(NULL) == 0);
    TEST("NULL 查找元素返回 0", SeqList_GetOrdOfElem(NULL, 1) == 0);
    TEST("NULL 插入 (应失败)", !SeqList_InsertElem(NULL, 1, 1));
    TEST("NULL 获取 (应失败)", !SeqList_GetElem(NULL, 1, &elem));
    TEST("NULL 修改 (应失败)", !SeqList_PutElem(NULL, 1, 1, NULL));
    TEST("NULL 删除 (应失败)", !SeqList_DeleteElem(NULL, 1, &elem));

    // 空表操作
    SeqList_Destroy(&seq);
    SeqList_Initialize(&seq);
    TEST("空表查找返回 0", SeqList_GetOrdOfElem(&seq, 1) == 0);

    // 可选参数测试（elem / oldElem 可为 NULL）
    TEST("获取元素但 elem 为 NULL (空表越界，应失败)", !SeqList_GetElem(&seq, 1, NULL));
    SeqList_InsertElem(&seq, 1, 42);
    TEST("获取元素但 elem 为 NULL (有效位序)", SeqList_GetElem(&seq, 1, NULL));
    TEST("修改元素但 oldElem 为 NULL", SeqList_PutElem(&seq, 1, 99, NULL));
    TEST("删除元素但 elem 为 NULL", SeqList_DeleteElem(&seq, 1, NULL));

    SeqList_Destroy(&seq);
    return 0;
}

int test_expansion()
{
    printf("\n=== 扩容测试 ===\n");

    SeqList seq;
    SeqList_Initialize(&seq);

    // 插入超过初始容量 (16) 的元素，ord 是位序（从1开始）
    printf("  插入 20 个元素触发扩容...\n");
    for (int ord = 1; ord <= 20; ord++)
    {
        if (!SeqList_InsertElem(&seq, ord, ord * 10))
        {
            printf("  插入第 %d 个元素失败\n", ord);
            return 1;
        }
    }

    TEST("扩容后长度为 20", SeqList_GetLength(&seq) == 20);
    TEST("扩容后最大容量 >= 20", seq.MaxSize >= 20);

    // 验证所有元素正确
    int all_correct = 1;
    for (int ord = 1; ord <= 20; ord++)
    {
        int val;
        SeqList_GetElem(&seq, ord, &val);
        if (val != ord * 10)
        {
            all_correct = 0;
            break;
        }
    }
    TEST("扩容后所有元素正确", all_correct);

    printf("  扩容后的顺序表 (前 5 个和后 5 个):\n");
    printf("  [");
    for (int ord = 1; ord <= 5; ord++)
    {
        int val;
        SeqList_GetElem(&seq, ord, &val);
        printf("(%d)=%d, ", ord, val);
    }
    printf("... ");
    for (int ord = 16; ord <= 20; ord++)
    {
        int val;
        SeqList_GetElem(&seq, ord, &val);
        printf("(%d)=%d%s", ord, val, ord < 20 ? ", " : "");
    }
    printf("]\n");

    SeqList_Destroy(&seq);
    return 0;
}

int test_complex_scenarios()
{
    printf("\n=== 复杂场景测试 ===\n");

    SeqList seq;
    SeqList_Initialize(&seq);
    int elem;

    // 场景：在头部反复插入，ord 是位序（从1开始）
    printf("  测试：头部插入 5 次\n");
    for (int ord = 1; ord <= 5; ord++)
    {
        SeqList_InsertElem(&seq, 1, ord);
    }
    // 结果应该是 5, 4, 3, 2, 1
    TEST("头部插入后长度为 5", SeqList_GetLength(&seq) == 5);
    SeqList_GetElem(&seq, 1, &elem);
    TEST("第 1 个是 5", elem == 5);
    SeqList_GetElem(&seq, 5, &elem);
    TEST("第 5 个是 1", elem == 1);

    // 场景：在尾部插入
    printf("  测试：尾部追加 3 次\n");
    int len = SeqList_GetLength(&seq);
    SeqList_InsertElem(&seq, len + 1, 100);
    SeqList_InsertElem(&seq, len + 2, 200);
    SeqList_InsertElem(&seq, len + 3, 300);
    TEST("尾部追加后长度为 8", SeqList_GetLength(&seq) == 8);

    // 场景：在中间插入
    printf("  测试：在中间位序插入\n");
    SeqList_InsertElem(&seq, 4, 999); // 在位序 3 和 2 之间插入 999
    SeqList_GetElem(&seq, 3, &elem);
    TEST("第 3 个是 3", elem == 3);
    SeqList_GetElem(&seq, 4, &elem);
    TEST("第 4 个是 999", elem == 999);
    SeqList_GetElem(&seq, 5, &elem);
    TEST("第 5 个是 2", elem == 2);

    printf("  当前顺序表内容:\n");
    SeqList_Print(&seq);

    // 场景：删除头部、尾部、中间
    printf("  测试：删除头部\n");
    SeqList_DeleteElem(&seq, 1, &elem);
    TEST("删除头部元素是 5", elem == 5);
    TEST("新头部是 4", SeqList_GetElem(&seq, 1, &elem) && elem == 4);

    printf("  测试：删除尾部\n");
    int old_len = SeqList_GetLength(&seq);
    SeqList_DeleteElem(&seq, old_len, &elem);
    TEST("删除尾部元素是 300", elem == 300);

    printf("  测试：删除中间 (位序 3)\n");
    SeqList_DeleteElem(&seq, 3, &elem);
    TEST("删除的元素是 999", elem == 999);

    printf("  操作后顺序表内容:\n");
    SeqList_Print(&seq);

    SeqList_Destroy(&seq);
    return 0;
}

int main()
{
    printf("=======================================\n");
    printf("       顺序表 (SeqList) 测试程序       \n");
    printf("=======================================\n");

    int result = 0;

    result |= test_basic_operations();
    result |= test_boundary_conditions();
    result |= test_expansion();
    result |= test_complex_scenarios();

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
