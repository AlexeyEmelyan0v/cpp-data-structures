from random import randint, choice
from fractions import Fraction as fr
import sys
from math import fabs

sys.set_int_max_str_digits(10 ** 8)

kBasesDegree = 9
kBase = 1000000000


def initializer_for_big(a: int) -> str:
    return '{' + ', '.join(map(str, map(int, (('-' if a < 0 else '') + (' ' + ('-' if a < 0 else '')).join(reversed(
        [''.join(reversed(list(reversed(str(abs(a))))[x:x + kBasesDegree])) for x in
         range(0, len(str(abs(a))), kBasesDegree)]))).split()))) + '}'


def gen_int() -> int:
    return randint(-(10 ** 9), 10 ** 9)


def gen_big() -> int:
    max_deg = 10  # for base
    deg = randint(1, max_deg)
    return randint(-(kBase ** deg), +(kBase ** deg))


def gen_name() -> str:
    length = 6
    power_of_alphabet = 26
    res = ""
    for i in range(length):
        res += choice(list("qwertyuiopasdfghjklzxcvbnm"[:power_of_alphabet:]))
    return res


out_file = open('main.cpp', 'w')
out_file_suggestion = open('suggested_output.txt', 'w')

spaces = 0


def ident():
    global spaces
    return spaces * " "


kOneIdent = 2


def ident_up():
    global spaces
    spaces += kOneIdent


def ident_down():
    global spaces
    spaces -= kOneIdent


def out_cpp(*args, **kwargs):
    print(ident(), end='', file=out_file)
    print(*args, **kwargs, file=out_file)


def out_sgg(*args, **kwargs):
    print(*args, **kwargs, file=out_file_suggestion)


def write_cpp(line: str):
    out_cpp(line, end='')


def start_namespace():
    out_cpp('{')
    ident_up()


def end_namespace():
    ident_down()
    out_cpp('}')


def out_includes():
    out_cpp('#include <iostream>')
    out_cpp('#include \"biginteger.h\"')
    out_cpp()


def write_main():
    write_cpp('int main()')
    start_namespace()


integers = dict()


def cpp_div(num1: int, num2: int) -> int:
    return (abs(num1) // abs(num2)) * (1 if num1 * num2 > 0 else -1)


def cpp_mod(num1: int, num2: int) -> int:
    return (abs(num1) % abs(num2)) * (1 if num1 > 0 else -1)


def cpp_add(num1: int, num2: int) -> int:
    return num1 + num2


def cpp_sub(num1: int, num2: int) -> int:
    return num1 - num2


def cpp_mul(num1: int, num2: int) -> int:
    return num1 * num2


def f1(name: str):
    out_cpp(f"{name} = -{name};")
    out_cpp(f"std::cout << {name} << std::endl;")
    integers[name] = -integers[name]
    out_sgg(integers[name])


def f2(name: str):
    out_cpp(f"std::cout << --{name} << std::endl;")
    integers[name] -= 1
    out_sgg(integers[name])


def f3(name: str):
    out_cpp(f"std::cout << {name}-- << std::endl;")
    out_sgg(integers[name])
    integers[name] -= 1


def f2(name: str):
    out_cpp(f"std::cout << --{name} << std::endl;")
    integers[name] -= 1
    out_sgg(integers[name])


def f3(name: str):
    out_cpp(f"std::cout << {name}-- << std::endl;")
    out_sgg(integers[name])
    integers[name] -= 1


def f4(name: str):
    out_cpp(f"std::cout << ++{name} << std::endl;")
    integers[name] += 1
    out_sgg(integers[name])


def f5(name: str):
    out_cpp(f"std::cout << {name}++ << std::endl;")
    out_sgg(integers[name])
    integers[name] += 1


operations_int = {'+': cpp_add, '-': cpp_sub, '*': cpp_mul, '%': cpp_mod, '/': cpp_div}


def cut(name_1: str):
    max_deg_base = 20
    while abs(integers[name_1]) > (kBase ** max_deg_base):
        integers[name_1] = (abs(integers[name_1]) // abs(kBase)) * (1 if integers[name_1] > 0 else -1)
        out_cpp(f"{name_1} /= {kBase}{'_bi' * choice([0, 1])};")
        out_cpp(f"std::cout << {name_1}{'.toString()' * choice([0, 1])} << std::endl;")
        out_sgg(integers[name_1])


def operation_out_int(name_1: str, name_2: str, op: str):
    if op in '/%' and integers[name_2] == 0:
        return
    out_cpp(f"std::cout << ({name_1} {op} {name_2}){'.toString()' * choice([0, 1])} << std::endl;")
    out_sgg(operations_int[op](integers[name_1], integers[name_2]))


def operation_int(name_1: str, name_2: str, op: str):
    if (op in '/%' and integers[name_2] == 0) or name_1 == name_2:
        return
    out_cpp(f"{name_1} {op}= {name_2};")
    integers.update({name_1: operations_int[op](integers[name_1], integers[name_2])})
    out_cpp(f"std::cout << {name_1} << std::endl;")
    out_sgg(integers[name_1])
    if op in '/':
        cut(name_1)


def generate_int():
    name = gen_name()
    value = gen_int()
    if name in integers or name in frac:
        return
    out_cpp(f"BigInteger {name} = {value};")
    out_cpp(f"std::cout << {name} << std::endl;")
    integers.update({name: value})
    out_sgg(integers[name])


def generate_big_int():
    name = gen_name()
    value = gen_big()
    if name in integers or name in frac:
        return
    out_cpp(f"BigInteger {name} = {initializer_for_big(value)};")
    out_cpp(f"std::cout << {name} << std::endl;")
    integers.update({name: value})
    out_sgg(integers[name])


frac = dict()


def generate_frac():
    name = gen_name()
    value = fr(gen_int())
    if name in integers or name in frac:
        return
    out_cpp(f"Rational {name} = {value};")
    out_cpp(f"std::cout << {name}.toString() << std::endl;")
    frac.update({name: value})
    out_sgg(frac[name])


def cut_frac(name: str):
    max_deg_base = 20
    if abs(frac[name].numerator) > (kBase ** max_deg_base) or abs(frac[name].denominator) > (kBase ** max_deg_base):
        frac[name] = fr(gen_int())
        out_cpp(f"{name} = {frac[name]};")
        out_cpp(f"std::cout << {name}.toString() << std::endl;")
        out_sgg(frac[name])


def operation_frac(name_1: str, name_2: str, op: str):
    if (op in '/%' and frac[name_2] == 0) or name_1 == name_2:
        return
    out_cpp(f"{name_1} {op}= {name_2};")
    frac.update({name_1: eval(f"frac['{name_1}'] {op} frac['{name_2}']")})
    out_cpp(f"std::cout << {name_1}.toString() << std::endl;")
    precision = randint(0, 10)
    if fabs(frac[name_1]) < 1:
        out_cpp(f"std::cout << {name_1}.asDecimal({precision}) << std::endl;")
    out_sgg(frac[name_1])
    if fabs(frac[name_1]) < 1:
        out_sgg('{:.{exp}f}'.format(float(frac[name_1]), exp=precision))
    if op in '*/':
        cut_frac(name_1)


def operation_out_frac(name_1: str, name_2: str, op: str):
    if op in '/%' and frac[name_2] == 0:
        return
    out_cpp(f"std::cout << ({name_1} {op} {name_2}).toString() << std::endl;")
    out_sgg(eval(f"frac['{name_1}'] {op} frac['{name_2}']"))


def compare(bb, name_1: str, name_2: str, op: str):
    out_cpp(f"std::cout << ({name_1} {op} {name_2}) << std::endl;")
    out_sgg(1 if eval(f"{bb}['{name_1}'] {op} {bb}['{name_2}']") else 0)


if __name__ == "__main__":
    number_of_instructions = 100
    out_includes()
    write_main()

    start_namespace()
    for i in range(20):
        generate_int()
    for _ in range(number_of_instructions):
        choice([operation_int, operation_out_int])(choice(list(integers.keys())), choice(list(integers)),
                                                   choice('+-*/%'))
        choice([f1, f2, f3, f4, f5])(choice(list(integers.keys())))
        compare('integers', choice(list(integers.keys())), choice(list(integers.keys())),
                choice(['==', '!=', '>', '<', '>=', '<=']))
    end_namespace()

    start_namespace()
    for i in range(20):
        generate_frac()
    for _ in range(number_of_instructions):
        choice([operation_frac, operation_out_frac])(choice(list(frac)), choice(list(frac)), choice('+-/*'))
        compare('frac', choice(list(frac.keys())), choice(list(frac.keys())),
                choice(['==', '!=', '>', '<', '>=', '<=']))
    end_namespace()
    end_namespace()
