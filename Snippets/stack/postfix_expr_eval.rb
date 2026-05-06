# frozen_string_literal: true

# ==========================================================================
# 后缀表达式求值（逆波兰表示法，RPN）
#
# 后缀表达式（如 "1 2 3 * +"）的求值非常直接：
# 从左到右扫描 token，遇到操作数则压栈，遇到运算符则弹出栈顶两个操作数，
# 计算后再压回栈。扫描结束，栈中剩下的唯一值就是结果。
#
# 不需要括号，也不需要优先级表——这正是后缀表达式的优势所在。
#
# --------------------------------------------------------------------------
# 算法流程（以求值 "1 2 3 * +" 为例）：
#
#   token  栈（栈顶在右）          操作
#   -----  ----------------------  --------------------
#   '1'    [1]                    操作数入栈
#   '2'    [1, 2]                 操作数入栈
#   '3'    [1, 2, 3]              操作数入栈
#   '*'    [1, 6]                 弹出 3, 2 → 2*3=6 入栈
#   '+'    [7]                    弹出 6, 1 → 1+6=7 入栈
#   结束   → 结果 = 7
#
# 注意：运算符弹出操作数的顺序是「先右后左」（先弹的是右操作数），
#       这对加法和乘法无影响，但对减法和除法至关重要。
#
# --------------------------------------------------------------------------
# 复杂度：
#   时间复杂度 O(n) —— 每个 token 处理一次
#   空间复杂度 O(n) —— 操作数栈的大小
# ==========================================================================

# 对后缀表达式求值，返回计算结果
#
# @param expr [String] 以空格分隔的后缀表达式，如 "1 2 3 * +"
# @return [Integer] 计算结果
def eval_postfix(expr)
  stack = []

  # 按空格拆分为 token 序列
  expr.split.each do |token|
    case token
    when '*', '/', '+', '-'
      # 弹出右操作数和左操作数（顺序很重要！先弹右后弹左）
      right = stack.pop
      left  = stack.pop

      result = case token
               when '+' then left + right
               when '-' then left - right
               when '*' then left * right
               when '/' then left / right # Ruby 整数除法自动取整
               end

      stack.push(result)
    else
      # 操作数：转换为整数后入栈
      stack.push(token.to_i)
    end
  end

  # 扫描结束，栈顶即为最终结果
  stack.pop
end

# ==========================================================================
# 单元测试
# ==========================================================================

require 'minitest/autorun'
require_relative 'infix_expr_to_postfix'

class TestEvalPostfix < Minitest::Test
  # ------------------------------------------------------------------
  # 基本二元运算（直接给后缀表达式）
  # ------------------------------------------------------------------
  def test_simple_addition
    assert_equal 3, eval_postfix('1 2 +')
  end

  def test_simple_subtraction
    assert_equal 2, eval_postfix('3 1 -')
  end

  def test_simple_multiplication
    assert_equal 20, eval_postfix('4 5 *')
  end

  def test_simple_division
    assert_equal 4, eval_postfix('8 2 /')
  end

  def test_division_truncation
    # Ruby 整数除法：7/2 = 3
    assert_equal 3, eval_postfix('7 2 /')
  end

  # ------------------------------------------------------------------
  # 运算符优先级已在转换阶段处理，求值阶段直接计算
  # ------------------------------------------------------------------
  def test_multiplication_before_addition
    # 1 + 2 * 3 = 7
    assert_equal 7, eval_postfix('1 2 3 * +')
  end

  def test_division_before_subtraction
    # 10 - 6 / 2 = 7
    assert_equal 7, eval_postfix('10 6 2 / -')
  end

  def test_same_precedence_left_to_right
    # 1 - 2 + 3 = 2
    assert_equal 2, eval_postfix('1 2 - 3 +')
  end

  def test_multiplication_division_left_to_right
    # 12 / 3 * 2 = 8
    assert_equal 8, eval_postfix('12 3 / 2 *')
  end

  # ------------------------------------------------------------------
  # 括号的影响（后缀中已无括号，但其效果体现在 token 顺序中）
  # ------------------------------------------------------------------
  def test_parentheses_effect
    # (1 + 2) * 3 = 9
    assert_equal 9, eval_postfix('1 2 + 3 *')
  end

  def test_nested_parentheses_effect
    # 1 * (2 + 3) = 5
    assert_equal 5, eval_postfix('1 2 3 + *')
  end

  # ------------------------------------------------------------------
  # 负数和零
  # ------------------------------------------------------------------
  def test_result_is_negative
    assert_equal(-1, eval_postfix('3 4 -'))
  end

  def test_result_is_zero
    assert_equal 0, eval_postfix('5 5 -')
  end

  # ------------------------------------------------------------------
  # 边界情况
  # ------------------------------------------------------------------
  def test_single_operand
    assert_equal 42, eval_postfix('42')
  end

  def test_multiple_digit_operands
    assert_equal 579, eval_postfix('123 456 +')
  end

  # ------------------------------------------------------------------
  # 集成测试：中缀 → 后缀 → 求值 一条龙
  # ------------------------------------------------------------------
  def test_integration_simple
    assert_equal 3, eval_postfix(to_postfix('1+2'))
  end

  def test_integration_with_precedence
    assert_equal 7, eval_postfix(to_postfix('1+2*3'))
  end

  def test_integration_with_parentheses
    assert_equal 9, eval_postfix(to_postfix('(1+2)*3'))
  end

  def test_integration_complex
    # ((15/(7-(1+1)))*3)-(2+(1+1)) = 5
    assert_equal 5, eval_postfix(to_postfix('((15/(7-(1+1)))*3)-(2+(1+1))'))
  end

  def test_integration_another_complex
    # 5 + ((1 + 2) * 4) - 3 = 14
    assert_equal 14, eval_postfix(to_postfix('5+((1+2)*4)-3'))
  end

  def test_integration_all_four_operators
    # 1 + 2 * 3 - 4 / 5 = 1 + 6 - 0 = 7
    assert_equal 7, eval_postfix(to_postfix('1+2*3-4/5'))
  end

  def test_integration_multi_digit
    assert_equal 579, eval_postfix(to_postfix('123+456'))
  end
end
