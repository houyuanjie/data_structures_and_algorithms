#include <stdio.h>
#include <assert.h>
#include "seq_list.h"

// 测试宏：检查返回值并打印结果
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

// 测试宏：检查返回码
#define TEST_RESULT(name, result, expected) \
    TEST(name, (result) == (expected))

int test_basic_operations()
{
    printf("\n=== 基本操作测试 ===\n");

    SeqList seq;
    int item;

    // 初始化
    TEST_RESULT("初始化顺序表", SeqList_Initialize(&seq), SeqList_Success);
    TEST("初始长度为 0", SeqList_GetLength(&seq) == 0);
    TEST("初始为空表", SeqList_IsEmpty(&seq));

    // 插入测试
    TEST_RESULT("在位置 1 插入 10", SeqList_InsertItem(&seq, 1, 10), SeqList_Success);
    TEST_RESULT("在位置 2 插入 20", SeqList_InsertItem(&seq, 2, 20), SeqList_Success);
    TEST_RESULT("在位置 1 插入 5 (头部)", SeqList_InsertItem(&seq, 1, 5), SeqList_Success);
    TEST("当前长度为 3", SeqList_GetLength(&seq) == 3);

    // 打印当前状态
    printf("  当前顺序表内容:\n");
    SeqList_Print(&seq);

    // 获取测试
    TEST_RESULT("获取位序 1 的元素", SeqList_GetItem(&seq, 1, &item), SeqList_Success);
    TEST("位序 1 的元素是 5", item == 5);

    TEST_RESULT("获取位序 2 的元素", SeqList_GetItem(&seq, 2, &item), SeqList_Success);
    TEST("位序 2 的元素是 10", item == 10);

    TEST_RESULT("获取位序 3 的元素", SeqList_GetItem(&seq, 3, &item), SeqList_Success);
    TEST("位序 3 的元素是 20", item == 20);

    // 修改测试
    int oldItem;
    TEST_RESULT("修改位序 2 的元素为 15", SeqList_PutItem(&seq, 2, 15, &oldItem), SeqList_Success);
    TEST("旧值为 10", oldItem == 10);
    TEST_RESULT("获取修改后的值", SeqList_GetItem(&seq, 2, &item), SeqList_Success);
    TEST("新值为 15", item == 15);

    // 查找测试
    TEST("查找 5 的位序为 1", SeqList_IndexOfItem(&seq, 5) == 1);
    TEST("查找 15 的位序为 2", SeqList_IndexOfItem(&seq, 15) == 2);
    TEST("查找 20 的位序为 3", SeqList_IndexOfItem(&seq, 20) == 3);
    TEST("查找不存在的元素返回 -3", SeqList_IndexOfItem(&seq, 999) == SeqList_Error_ITEM_NOT_FOUND);

    // 删除测试
    TEST_RESULT("删除位序 2 的元素", SeqList_DeleteItem(&seq, 2, &item), SeqList_Success);
    TEST("删除的元素是 15", item == 15);
    TEST("删除后长度为 2", SeqList_GetLength(&seq) == 2);

    TEST_RESULT("获取新的位序 2", SeqList_GetItem(&seq, 2, &item), SeqList_Success);
    TEST("现在位序 2 的元素是 20", item == 20);

    // 销毁
    TEST_RESULT("销毁顺序表", SeqList_Destroy(&seq), SeqList_Success);
    TEST("销毁后为空表", SeqList_IsEmpty(&seq));

    return 0;
}

int test_boundary_conditions()
{
    printf("\n=== 边界条件测试 ===\n");

    SeqList seq;
    int item;

    SeqList_Initialize(&seq);

    // 越界插入测试
    TEST_RESULT("在位置 0 插入 (越界)", SeqList_InsertItem(&seq, 0, 1), SeqList_Error_INDEX_OUT_OF_RANGE);
    TEST_RESULT("在位置 2 插入空表 (越界)", SeqList_InsertItem(&seq, 2, 1), SeqList_Error_INDEX_OUT_OF_RANGE);

    // 插入一个元素后测试越界
    SeqList_InsertItem(&seq, 1, 100);
    TEST_RESULT("在位置 3 插入 (越界)", SeqList_InsertItem(&seq, 3, 1), SeqList_Error_INDEX_OUT_OF_RANGE);

    // 越界获取/修改/删除
    TEST_RESULT("获取位序 0 (越界)", SeqList_GetItem(&seq, 0, &item), SeqList_Error_INDEX_OUT_OF_RANGE);
    TEST_RESULT("获取位序 2 (越界)", SeqList_GetItem(&seq, 2, &item), SeqList_Error_INDEX_OUT_OF_RANGE);
    TEST_RESULT("修改位序 0 (越界)", SeqList_PutItem(&seq, 0, 1, NULL), SeqList_Error_INDEX_OUT_OF_RANGE);
    TEST_RESULT("删除位序 0 (越界)", SeqList_DeleteItem(&seq, 0, &item), SeqList_Error_INDEX_OUT_OF_RANGE);

    // NULL 指针测试
    TEST_RESULT("NULL 初始化", SeqList_Initialize(NULL), SeqList_Error_NULL_SEQ);
    TEST_RESULT("NULL 销毁", SeqList_Destroy(NULL), SeqList_Error_NULL_SEQ);
    TEST("NULL 获取长度", SeqList_GetLength(NULL) == SeqList_Error_NULL_SEQ);
    TEST("NULL 查找元素", SeqList_IndexOfItem(NULL, 1) == SeqList_Error_NULL_SEQ);
    TEST_RESULT("NULL 插入", SeqList_InsertItem(NULL, 1, 1), SeqList_Error_NULL_SEQ);
    TEST_RESULT("NULL 获取", SeqList_GetItem(NULL, 1, &item), SeqList_Error_NULL_SEQ);
    TEST_RESULT("NULL 修改", SeqList_PutItem(NULL, 1, 1, NULL), SeqList_Error_NULL_SEQ);
    TEST_RESULT("NULL 删除", SeqList_DeleteItem(NULL, 1, &item), SeqList_Error_NULL_SEQ);

    // 空表操作
    SeqList_Destroy(&seq);
    SeqList_Initialize(&seq);
    TEST("空表查找返回未找到", SeqList_IndexOfItem(&seq, 1) == SeqList_Error_ITEM_NOT_FOUND);

    // 可选参数测试（item 可为 NULL）
    TEST_RESULT("获取元素但 item 为 NULL", SeqList_GetItem(&seq, 1, NULL), SeqList_Error_INDEX_OUT_OF_RANGE); // 先插入一个
    SeqList_InsertItem(&seq, 1, 42);
    TEST_RESULT("获取元素但 item 为 NULL (有效索引)", SeqList_GetItem(&seq, 1, NULL), SeqList_Success);
    TEST_RESULT("修改元素但 oldItem 为 NULL", SeqList_PutItem(&seq, 1, 99, NULL), SeqList_Success);
    TEST_RESULT("删除元素但 item 为 NULL", SeqList_DeleteItem(&seq, 1, NULL), SeqList_Success);

    SeqList_Destroy(&seq);
    return 0;
}

int test_expansion()
{
    printf("\n=== 扩容测试 ===\n");

    SeqList seq;
    SeqList_Initialize(&seq);

    // 插入超过初始容量 (16) 的元素
    printf("  插入 20 个元素触发扩容...\n");
    for (int i = 1; i <= 20; i++)
    {
        SeqList_Result r = SeqList_InsertItem(&seq, i, i * 10);
        if (r != SeqList_Success)
        {
            printf("  插入第 %d 个元素失败，返回码: %d\n", i, r);
            return 1;
        }
    }

    TEST("扩容后长度为 20", SeqList_GetLength(&seq) == 20);
    TEST("扩容后最大容量 >= 20", seq.MaxSize >= 20);

    // 验证所有元素正确
    int all_correct = 1;
    for (int i = 1; i <= 20; i++)
    {
        int val;
        SeqList_GetItem(&seq, i, &val);
        if (val != i * 10)
        {
            all_correct = 0;
            break;
        }
    }
    TEST("扩容后所有元素正确", all_correct);

    printf("  扩容后的顺序表 (前 5 个和后 5 个):\n");
    printf("  [");
    for (int i = 1; i <= 5; i++)
    {
        int val;
        SeqList_GetItem(&seq, i, &val);
        printf("(%d)=%d, ", i, val);
    }
    printf("... ");
    for (int i = 16; i <= 20; i++)
    {
        int val;
        SeqList_GetItem(&seq, i, &val);
        printf("(%d)=%d%s", i, val, i < 20 ? ", " : "");
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
    int item;

    // 场景：在头部反复插入
    printf("  测试：头部插入 5 次\n");
    for (int i = 1; i <= 5; i++)
    {
        SeqList_InsertItem(&seq, 1, i);
    }
    // 结果应该是 5, 4, 3, 2, 1
    TEST("头部插入后长度为 5", SeqList_GetLength(&seq) == 5);
    SeqList_GetItem(&seq, 1, &item);
    TEST("第 1 个是 5", item == 5);
    SeqList_GetItem(&seq, 5, &item);
    TEST("第 5 个是 1", item == 1);

    // 场景：在尾部插入
    printf("  测试：尾部追加 3 次\n");
    int len = SeqList_GetLength(&seq);
    SeqList_InsertItem(&seq, len + 1, 100);
    SeqList_InsertItem(&seq, len + 2, 200);
    SeqList_InsertItem(&seq, len + 3, 300);
    TEST("尾部追加后长度为 8", SeqList_GetLength(&seq) == 8);

    // 场景：在中间插入
    printf("  测试：在中间位置插入\n");
    SeqList_InsertItem(&seq, 4, 999); // 在 3 和 2 之间插入 999
    SeqList_GetItem(&seq, 3, &item);
    TEST("第 3 个是 3", item == 3);
    SeqList_GetItem(&seq, 4, &item);
    TEST("第 4 个是 999", item == 999);
    SeqList_GetItem(&seq, 5, &item);
    TEST("第 5 个是 2", item == 2);

    printf("  当前顺序表内容:\n");
    SeqList_Print(&seq);

    // 场景：删除头部、尾部、中间
    printf("  测试：删除头部\n");
    SeqList_DeleteItem(&seq, 1, &item);
    TEST("删除头部元素是 5", item == 5);
    TEST("新头部是 4", SeqList_GetItem(&seq, 1, &item) == SeqList_Success && item == 4);

    printf("  测试：删除尾部\n");
    int old_len = SeqList_GetLength(&seq);
    SeqList_DeleteItem(&seq, old_len, &item);
    TEST("删除尾部元素是 300", item == 300);

    printf("  测试：删除中间 (位置 3)\n");
    SeqList_DeleteItem(&seq, 3, &item);
    TEST("删除的元素是 999", item == 999);

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