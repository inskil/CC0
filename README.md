# c0编译器

## 使用说明
``` shell script
Usage: cc0 [options] input

Positional arguments:
input           speicify the file to be compiled.

Optional arguments:
-h --help       show this help message and exit
-t              perform tokenization for the input file to text file.
-s              perform syntactic analysis for the input file to text file.
-c              perform syntactic analysis for the input file to binary file.
-o --output     specify the output file.
-r              Run you input file directly.
```
- -h 调出帮助
- -t 进行词法分析，输出文本文件
- -s 进行语法分析，输出文本文件
- -c 进行语法分析，输出二进制文件
- -o file , 输出文件
    - 针对 -t 默认输出到out文件 ，-o 定义则输出到file
    - 针对 -s 默认输出到out文件，-o 定义则输出到file
    - 针对 -c 默认输出到out文件，且生产一个名为cache的文本文件
- -r 直接跑符合文法的代码，若任何一个过程出错，都报错
    - 无权定义输出流，默认全部输出到std::out
    - -o有效，但仅仅用于二进制文件名
    - 当不给出 -o 时，默认输出二进制到out文件，且生产一个名为cache的文本文件
    - 当给出 -o file 时，输出二进制到file文件，且生产一个名为cache的文本文件
    

## 出错处理
部分错误简化处理。