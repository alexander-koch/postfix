# PostFix

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

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

## Contributing

Feel free to file issues and send pull requests.
Contributions are highly welcome!

## License

Copyright (c) Alexander Koch 2018

Licensed under [GNU General Public License Version 3](LICENSE).