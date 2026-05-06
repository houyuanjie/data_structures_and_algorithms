# frozen_string_literal: true

# ==========================================================================
# 中缀表达式 → 后缀表达式（逆波兰表示法，RPN）
#
# 算法：调度场算法（Shunting Yard Algorithm），由 Edsger Dijkstra 提出。
# 借助一个运算符栈，将人类习惯的中缀表达式转换为计算机易于求值的后缀表达式。
#
# --------------------------------------------------------------------------
# 算法流程（以表达式 "1 + 2 * 3" 为例）：
#
#   输入: 1  +  2  *  3
#         |  |  |  |  |
#   操作数 → 直接输出到结果串
#   运算符 → 与栈顶运算符比较优先级：
#            若栈顶优先级 >= 当前优先级，则弹栈输出，重复比较；
#            否则当前运算符入栈。
#   '('    → 直接入栈
#   ')'    → 弹栈输出直到遇到 '('，丢弃括号对
#   结尾    → 将栈中剩余运算符全部弹栈输出
#
#   处理 "1+2*3" 的推演：
#     读 '1'   输出: 1          栈: []
#     读 '+'   输出: 1          栈: [+]
#     读 '2'   输出: 1 2        栈: [+]
#     读 '*'   输出: 1 2        栈: [+, *]   ( '*' 优先级 > '+'，入栈)
#     读 '3'   输出: 1 2 3      栈: [+, *]
#     结束     输出: 1 2 3 * +  栈: []       (弹栈：先 * 后 +)
#
# --------------------------------------------------------------------------
# 复杂度：
#   时间复杂度 O(n) —— 每个 token 均入栈、出栈各至多一次
#   空间复杂度 O(n) —— 输出数组及运算符栈的大小与输入规模成线性关系
# ==========================================================================

require 'prism'

# 将中缀表达式转换为后缀表达式（逆波兰表示法）
#
# 用 Ruby 内置的 Prism 词法分析器将输入字符串拆分为 token 序列，
# 再以调度场算法处理 token 序列，得到后缀表达式，token 之间以空格分隔。
#
# @param expr [String] 中缀算术表达式，如 "1 + 2 * 3"
# @return [String] 后缀表达式，各 token 以空格分隔，如 "1 2 3 * +"
def to_postfix(expr)
  # ---- 运算符优先级表 ----
  # 数值越大优先级越高；相同优先级按从左到右结合（在弹栈比较时用 >= 实现）
  precedence = { '*' => 2, '/' => 2, '+' => 1, '-' => 1 }

  ops = []                # 运算符栈
  postfix = []            # 结果数组（后缀表达式的 token 序列）

  # ---- 第 1 步：分词 ----
  # 利用 Ruby 的 Prism 词法分析器将表达式字符串拆分为 token
  tokens = Prism.lex(expr.strip).value       # [[token, ...], ...]
  tokens = tokens.map { it.first.value }     # 取每个 token 的字符串值
  tokens = tokens.reject { it.strip.empty? } # 剔除空白 token（如有）

  # ---- 第 2 步：遍历 token 序列，按调度场算法处理 ----
  until tokens.empty?
    case token = tokens.shift
    when '('
      # 左括号总是直接入栈
      ops << token

    when ')'
      # 遇到右括号则连续弹栈，直到弹出匹配的左括号 '(' 并丢弃它
      until ops.empty?
        case op = ops.pop
        when '(' then break
        else postfix << op
        end
      end

    when '*', '/', '+', '-'
      # 当前 token 是二元运算符：
      # 先弹栈中所有「优先级 >= 当前运算符」的运算符并输出，
      # 然后将当前运算符入栈。
      #
      # 弹栈终止条件（满足任一即停止）：
      #   (a) 栈空
      #   (b) 栈顶为 '('
      #   (c) 栈顶运算符优先级 < 当前运算符优先级
      until ops.empty?
        break if ops.last == '('
        break if precedence[ops.last] < precedence[token]

        postfix << ops.pop
      end

      ops << token

    else
      # 操作数：直接输出到结果数组
      postfix << token
    end
  end

  # ---- 第 3 步：清空栈 ----
  # 输入已读完，将栈中剩余运算符全部弹出
  postfix << ops.pop until ops.empty?

  # 用空格连接所有 token 得到后缀表达式字符串
  postfix.join(' ')
end

# ==========================================================================
# 单元测试
# ==========================================================================

require 'minitest/autorun'

class TestInfixExprToPostfix < Minitest::Test
  # ------------------------------------------------------------------
  # 基本二元运算
  # ------------------------------------------------------------------
  def test_simple_addition
    assert_equal '1 2 +', to_postfix('1+2')
  end

  def test_simple_subtraction
    assert_equal '3 1 -', to_postfix('3-1')
  end

  def test_simple_multiplication
    assert_equal '4 5 *', to_postfix('4*5')
  end

  def test_simple_division
    assert_equal '8 2 /', to_postfix('8/2')
  end

  # ------------------------------------------------------------------
  # 运算符优先级
  # ------------------------------------------------------------------
  def test_multiplication_before_addition
    # '*' 优先级高于 '+'，因此 '*' 先输出
    assert_equal '1 2 3 * +', to_postfix('1+2*3')
  end

  def test_division_before_subtraction
    assert_equal '10 6 2 / -', to_postfix('10-6/2')
  end

  def test_same_precedence_left_to_right
    # '+' 与 '-' 优先级相同，从左到右
    assert_equal '1 2 - 3 +', to_postfix('1-2+3')
  end

  def test_multiplication_division_left_to_right
    assert_equal '12 3 / 2 *', to_postfix('12/3*2')
  end

  # ------------------------------------------------------------------
  # 括号
  # ------------------------------------------------------------------
  def test_parentheses_change_precedence
    # 括号使 '+' 优先于 '*'
    assert_equal '1 2 + 3 *', to_postfix('(1+2)*3')
  end

  def test_nested_parentheses
    assert_equal '1 2 3 + *', to_postfix('1*(2+3)')
  end

  def test_double_nested_parentheses
    assert_equal '1 2 + 3 4 + *', to_postfix('(1+2)*(3+4)')
  end

  # ------------------------------------------------------------------
  # 复杂表达式
  # ------------------------------------------------------------------
  def test_complex_expression
    # 中缀: ((15/(7-(1+1)))*3)-(2+(1+1))
    assert_equal '15 7 1 1 + - / 3 * 2 1 1 + + -',
                 to_postfix('((15/(7-(1+1)))*3)-(2+(1+1))')
  end

  def test_another_complex_expression
    # 中缀: 5 + ((1 + 2) * 4) - 3
    assert_equal '5 1 2 + 4 * + 3 -',
                 to_postfix('5+((1+2)*4)-3')
  end

  # ------------------------------------------------------------------
  # 边界情况
  # ------------------------------------------------------------------
  def test_single_operand
    assert_equal '42', to_postfix('42')
  end

  def test_expression_with_spaces
    # 输入中包含空格应能正确处理（Prism 会忽略空格 token）
    assert_equal '1 2 +', to_postfix('1 + 2')
  end

  def test_all_four_operators
    assert_equal '1 2 3 * + 4 5 / -', to_postfix('1+2*3-4/5')
  end

  def test_redundant_parentheses
    assert_equal '1 2 +', to_postfix('((1+2))')
  end

  def test_multiple_digit_operands
    assert_equal '123 456 +', to_postfix('123+456')
  end

  def test_multiplication_before_multiple_additions
    # '*' 优先级高于 '+'，所以 2*3 先结合，等价于 (1+(2*3))+4
    assert_equal '1 2 3 * + 4 +', to_postfix('1+2*3+4')
  end
end
