# Lua Interpreter for RARS

Интерпретатор подмножества Lua, написанный на C и компилируемый в RISC-V ассемблер для запуска в симуляторе [RARS](https://github.com/TheThirdOne/rars).

## Зависимости

- `riscv64-elf-gcc` (bare-metal RISC-V GCC)
- Python 3
- Java (для запуска RARS)
- RARS `.jar` файл

## Сборка

```bash
python3 build.py
```

## Запуск 

```bash
java -jar rars1_6.jar program.s
```

## Реализованная грамматика

Подмножество [грамматики Lua 4.1](http://lua-users.org/wiki/LuaFourOneGrammar):

```
stmt    --> if_stmt | while_stmt | for_stmt

if_stmt --> IF expr THEN block [ ELSE block ] END

while_stmt --> WHILE expr DO block END

for_stmt --> FOR name IN expr DO block END

block   --> { stmt }

expr    --> primary | expr binop expr | unop expr | '(' expr ')'

primary --> number | string | name { suffix }

suffix  --> index | call

index   --> '[' expr ']' | '.' key

key     --> name

call    --> '(' [ exprs ] ')'

exprs   --> expr { ',' expr }

binop   --> '+' | '-' | '*' | '/' | '^' | '..' | AND | OR
          | '<' | '<=' | '>' | '>=' | '==' | '~='

unop    --> '-' | NOT
```