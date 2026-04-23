# frozen_string_literal: true

require 'prism'

# 中缀表达式转后缀表达式
#
# @param expr [String] 中缀表达式
# @return [String] 后缀表达式，空格分割
def to_postfix(expr)
  # 运算符优先级表，数值越大优先级越高
  precedence = { '*' => 2, '/' => 2, '+' => 1, '-' => 1 }

  # 转换中使用的运算符栈
  ops = []
  # 转换结果，后缀表达式
  postfix = String.new

  # 分词
  tokens = Prism.lex(expr.strip).value      # [[token, ...], ...]
  tokens = tokens.map { it.first.value }    # [str, ...]
  tokens = tokens.reject { it.strip.empty? }

  # 处理分词后的中缀表达式
  until tokens.empty?
    case token = tokens.shift
    when '('
      # 左括号：直接入栈
      ops << token

    when ')'
      # 右括号：不入栈，连续弹出栈顶的操作符加入后缀表达式，
      # 直到遇到左括号 '('，将其弹出并丢弃
      until ops.empty?
        case op = ops.pop
        when '('
          break
        else
          postfix << op
        end
      end

    when '*', '/', '+', '-'
      # 运算符：连续弹出栈中优先级更高或相同的符号，加入后缀表达式，
      # 直到空栈，或者碰到栈顶是 '(' ，或是更低优先级的符号，则停止弹栈
      until ops.empty?
        break if ops.last == '('
        break if precedence[ops.last] < precedence[token]

        postfix << ops.pop
      end

      # 弹栈后，将当前运算符入栈
      ops << token

    else
      # 操作数：直接加入后缀表达式
      postfix << token
    end

    postfix << ' ' unless postfix.end_with?(' ')
  end

  # 将栈中剩余运算符依次弹出，加入后缀表达式
  until ops.empty?
    postfix << ops.pop
    postfix << ' ' unless postfix.end_with?(' ')
  end

  postfix.strip
end

if $PROGRAM_NAME == __FILE__
  # 中缀表达式
  expr = '((15/(7-(1+1)))*3)-(2+(1+1))'
  puts "expr = #{expr}"

  postfix = to_postfix(expr)
  puts "postfix = #{postfix}"
end
