# PostFix

This is an interpreter for the PostFix programming language.

## Language

PostFix is a stack-based programming language.
It is inspired by Adobe Systems' PostScript as well as by Forth, Racket and NewLisp.

```
celsius-to-fahrenheit: (celsius :Int -> :Int) {
    celsius 9 * 5 i/ 32 +
} fun

"hello world!" println
24 celsius-to-fahrenheit println
```

### Further information and language tutorials

 * https://postfix.hci.uni-hannover.de/postfix-lang.html
 * https://hci.uni-hannover.de/files/prog1script-postfix/script.html

## Installation

To build the executable run

```sh
$ make
```

If you want to build the example library to test dynamic linking

```sh
$ make lib
```

This will build the example.so in the root directory of the project.
To load the library type the following in the interpreter:

```
>>> "example.so" load-library
```

## License

Copyright (c) Alexander Koch 2018