# Language Reference

Due to inherent limitations of the parallel parsing process and the novelty of this compiler, the syntax of the language is relatively simple. Lexical grammar is defined by `src/compiler/lexer/pareas.lex`, and the parsing grammar is defined by `src/compiler/parser/pareas.g`. In general, the language is designed to look like familiar programming languages like C, along with some improvements and creative workarounds around parser limitations.

```
fn gcd[a: int, b: int]: int {
    if b == 0 {
        return a;
    }
    return gcd[b, a % b];
}
```

## Comments & Whitespace

Pareas supports single-line comments, prefixed by `//`. Anything after the first `//` on a line is ignored. This text may be used to describe a computation or otherwise leave an informational note to the reader.
```
fn comments[]: void {
  // This text is ignored.
}
```

Whitespace consists of any sequence of characters made up of space (` `), tab (`\t`), carriage return (`\r`), and newline (`\n`). Its main significance is to separate tokens (the input `ab cd` is interpreted as token `ab` followed by token `cd`), but is otherwise ignored, and may be used to format the program in a readable manner.
```
fn whitespace[   whitespace:int,    is   : int, not: int,  important  :int ]: void {
     whitespace     *is     +      not
           important;
}
```

## Data Types

There are two primitive data types in Pareas: `int`, which represents the type of a signed 32-bit integer value, and `float`, which represents the type of a 32-bit float value. The `void` type is used to represent absence of a value in particular situations, for example, when a function does not return a value. Additionally, there are a number of internal data types: `int_ref` and `float_ref`, which represent writable L-values of `int` and `float` respectively. The internal data type `invalid` is used in the compiler to indicate that a particular node does not produce a value at all, or that a type error occured. It cannot be written.

## Expressions

Pareas supports a precedence-based expression syntax, written using infix notation. These may appear as free-standing statements, in which case they are terminated using `;`, or as part of another construct, such as a sub-expression or condition expression of another statement.

### Literals

There are two types of numeric literals in Pareas: integers and floats. Integers are written in base-10 notation with no other characters; there is as of yet no support for other bases. There is no limit on the number of characters that may appear in an integer literal, however, the value of any integer literal which base-10 interpretation does not fit in 2^31-1 is undefined. Floating point literals are written using two base-10 integer literals separated by a dot. The former integer represents the integer part of the floating point value, and the latter represents the fraction part. Both parts must consist of at least one digit. Floating point parsing is handled in a very rudimentary way, and so the actual value of an interpreted floating point literal may differ a bit from the base-10 representation.

```
fn literals[]: void {
    var int_literal = 123;
    var float_literal = 123.456;
}
```

### Variables

Variables may be used to store the result of a particular computation, and re-use the value at a different point.

Variables may be declared using the syntax `var <name>: <type> = <expr>`, where `<type>` is either `int` or `float`. `<expr>` must be an expression which yields a value of the same type as `<type>`. `<name>` is a user-assigned identifier by which the same value may later be retrieved. Identifiers may consist of a sequence of alphanumeric characters, as well as `_`, with the exception that the first character is not a number.
```
fn variables[]: void {
    var an_int: int = 123; // Declare the variable `an_int`, which holds the value `123`.
    var another_int: int = an_int; // Declare the value `another_int`, and assign it the value that is currently in `an_int`.
    var a_float: float = 123.456; // Declare the value `a_float`, which holds the value `123.456`.

    // var 123: int = 123; // Invalid - name may not start with a number.
    // var not_an_int: int = 123.456; // Invalid - float value assigned to integer variable.
}
```

As a shorthand, the syntax `var <name> = <expr>` may be used to declare the variable `<name>` and infer the type of the variable simultaneously.
```
fn inferred_types[]: void {
    var an_int = 123; // `123` is an integer literal, so `an_int` holds an integer value.
    var a_float = 123.456; // `123.456` is a float literal, so `a_float` holds a float value.
    var another_int = an_int; // `an_int` holds an integer value, so `another_int` holds an integer type as well.
}
```

After a variable has been declared, it's value may be mutated by assigning to it, using the syntax `<name> = <expr>`. In this case, `<expr>` must yield a value of the same type as the variable has been declared with.
```
fn assignment[]: void {
    var an_int = 123;
    var another_int = an_int; // Create a new variable holding the same value as `an_int`.
    another_int = 456; // Re-assign a new value to `another_int`.
    // Note that `an_int` still holds 123 at this point.
}
```

Variables in pareas may be declared and assigned in expressions as well:
```
fn decl_in_expr[]: void {
    var a = 0;
    var c = (var b = a = 123) + 3;
    // `a` and `b` hold `123`, `c` holds `126`.
}
```

### Operators

Values may be operated upon through operators, which are written in infix notation in Pareas. Pareas supports the following operators:

| Syntax | Allowed types | Description            |
| ------ | ------------- | ---------------------- |
|   -x   | ints / floats | Negation               |
|   !x   | ints          | Logical negation       |
|   ~x   | ints          | Bitwise negation       |
|  x * y | ints / floats | Multiplication         |
|  x / y | ints / floats | Division               |
|  x % y | ints / floats | Remainder              |
|  x + y | ints / floats | Addition               |
|  x - y | ints / floats | Subtraction            |
| x >> y | ints          | Logical right shift    |
| x >>> y| ints          | Arithmetic right shift |
| x << y | ints          | Logial left shift      |
| x & y  | ints          | Bitwise AND            |
| x | y  | ints          | Bitwise OR             |
| x ^ y  | ints          | Bitwise XOR            |
| x == y | ints / floats | Equality               |
| x != y | ints / floats | Inequality             |
| x <= y | ints / floats | Less or equal than     |
| x < y  | ints / floats | Less than              |
| x >= y | ints / floats | Greater or equal than  |
| x > y  | ints / floats | Greater than           |
| x && y | ints          | Logical AND            |
| x || y | ints          | Logical OR             |
| x: T   | ints / floats | Type ascription        |
| a = x  | ints / floats | Assignment             |

When using a binary operator, both sides of the operator must yield a value of the
same type.
```
fn operators[]: void {
    var one = 1;
    var two = 2;
    var three = 1 + 2;
}
```

### Precedence

```
1: !x !y ~x
2: x*y x/y x%y
3: x+y x-y
4: x>>y x>>>y x<<y
5: x&y x|y x^y
6: x==y x!=y x<=y x<y x>=y x>y
7: x&&y
8: x||y
9: x:T
10: a=x
```

All operators are left-associative.

## Functions

The Pareas language allows the user to define multiple functions, which may be invoked from an expression. A function accepts a number of parameters which may be used to perform a computation, and may yield a value to the caller. The data type of each of these types must be declared along with the function declaration, and must be either `int` or `float`. If a function does not yield a value, it may declare `void` as the return type.

A function is declared using the syntax `fn <name>[<parameters...>]: <return type> { <statements...> }`. `<name>` is an identifier that may be used to invoke the function later on, and must be unique for each function in the program. Each `<parameter>` takes the form of `<name>: <type>`, and acts as a variable in the statement sequence making up the body of the function. Multiple parameters may be separated by a comma. `<statements...>` is a sequence of statements that implement the computation represented by the function. This may consist of semicolon-terminated expressions or control flow statements, which are executed from top to bottom.

To stop function execution and yield a value to the invoker, one may use the statement `return <expr>;`. All functions functions that are declared to return a value of a type other than `void` must end each possible code path of the function with a `return` statement. If early termination of a `void`-returning function is desired, one may omit the `<expr>`. In these functions, there is an implicit `return` statement at the end of the statement sequence making up the function body. Note that if a particular section of the function is statically determined to be unreachable - for example, if all possible code paths leading to that part of the function are already terminated using another `return` statement - it is not required to terminate that code path with another `return` statement.
```
fn add_two_ints[a: int, b: int]: int {
    return a + b;
}
```
```
fn returns_implicitly[]: void {
    // Not an error
}
```
```
fn forgotten_return[]: int {
    // Invalid - this function is supposed to return a value of type `int`, but it does not.
}
```
### Invocation

A function may be invoked using the syntax `<name>[<arguments...>]`. This expression invokes the function `<name>`, which must be supplied the right number of `<arguments...>`. Each argument is an expression that must yield a value of the type declared by the corresponding parameter in the function declaration. If a function is declared to return a value of type `int` or `float`, it may be used by the caller in further expressions. If the function is declared to return `void` instead, it cannot be used as no operation supports the `void` type. In this case, the function may only be invoked in an expression where the result is not used, such as a free-standing function call.
```
fn add_three_ints[a: int, b: int, c: int]: int {
    return add_two_ints[a, add_two_ints[b, c]];
}
```
```
fn returns_nothing[a: int, b: int]: void {
    // Does nothing...
}
fn also_does_nothing[]: void {
    returns_nothing[1, 2];
}
```

## Control Flow

Pareas supports two types of control flow: If-statements and while-statements.

### If-statements

If-statements may be used to redirect execution to either of two sequences of statements, depending on the result of an expression. These are written using the syntax `if <expr> { <statements...> } else { <statements... }`. If `<expr>`, which must yield a value of type `int`, yields a value other than 0, the first sequence of statements is executed. If the expression yields another value, then the second sequence of statements is executed. After either of the sequences of statements is executed, control-flow merges and continues with the next statement.

```
fn if_demo[a: int, b: int]: int {
    var c = 0;
    if a < b {
        c = 1;
    } else {
        c = 2;
    }
    // Independent of whether the if-part or else-part was selected, control-flow
    // continues here.
    return c;
}
```

If the else-part of an if-statement is not required, it may be omitted.
```
fn no_else[a: int, b: int]: int {
    if a < b {
        return 1;
    }
    return 2;
}
```

Furthermore, one may use the keyword `elif` as syntax suger over an if-statement where the only statement in the else-part is another if statement.
```
fn cmp[a: int, b: int]: int {
    if a < b {
        return -1;
    } elif a == b {
        return 0;
    } else {
        return 1;
    }

    // Equivalent to
    // if a < b {
    //     return -1;
    // } else {
    //     if a == b {
    //         return 0;
    //     } else {
    //         return 1;
    //     }
    // }

    // No return required here - all sub-sequences of the `if` statement return, and so this part is unreachable.
}
```

### While-statements

While-statements may be used to execute a particular sequence of statements multiple times, depending on the result of an expression. While-statements are written using the syntax `while <expr> { <statements...> }`. The sequence of statements is repeatedly executed as long as `<expr>` returns a value other than 0. Note that the condition is tested before the sequence of statements making up the body of the function is executed, and so if the first evaluation of `<expr>` yields 0, the body is not executed at all. If the `<expr>` ever reaches 0, control-flow continues with the next statement.

```
fn sum_of_n[n: int]: int {
    var sum = 0;
    var i = 1;
    while i <= n {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}
```
```
fn while_return[a: int]: int {
    while a {
        return 0;
    }
    // Invalid - `a` may yield 0, and so this part of the function is always reachable.
}
```

## Keywords

The language reserves the following keywords, which may not be used as variable or function names: `fn`, `if`, `else`, `elif`, `while`, `return`, `var`, `int`, `float`, and `void`.
