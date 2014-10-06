Jorvik Scheme
======

A simple cheme interpreter.  

<b>Requires:</b></br>
C++ 11 (with support for std::regex)</br>
Google Mock/Test for unit tests (submodules in the project)</br>

Details
--------
The aim here is a small, simple, scheme implementation.  I used it as a learning exercise for TDD, and to help me learn a bit more about
the lisp/scheme language.  Mostly it was useful for refreshing my language interpreter knowledge.  But also great fun to do.

Mostly inspired by Peter Norvig's Python Intepreter (improved version):  [lispy2](http://norvig.com/lispy2.html)</br>
I also took inspiration from Anthony Hay's conversion of Norvig's simpler version Scheme Interpreter [Lisp in 90 lines of c](http://howtowriteaprogram.blogspot.co.uk/2010/11/lisp-interpreter-in-90-lines-of-c.html)</br>

My version is intended to be much like Norvig's second version, with the additional error checking, support for booleans, etc.
I stopped short at adding the macro support, because I wanted to to the 'full' scheme template version, but haven't got around to it.  One day...
That stuff is pretty hard, and I don't have time.

The implementation is split pretty cleanly between tokenizing, parsing and interpreting.  It might be useful if you're trying to understand how that might be implemented.
I make no claim that this is either fast, lean or complete!

Unlike some implementations (such as Hay's), this interpereter uses linked lists of cells, instead of arrays.  That means that for the most part it supports 'dotted pairs', and will print them as such.
To make it fast, it uses a custom allocator to make the cells, and they are reasonably small units of memory (32 bytes).  They could be a little smaller with some effort too.

The tokenizer just splits up the input into known tokens, such as '(', '5', 'define', etc. </br>
The parser 'massages' the input cells to do things like convert 'define' to 'lambda', and various other things to make the intepreter's job easier, along with checking for syntax errors. </br>
The intepreter does the work of running the code, calling the functions, etc. </br>
The 'Evaluator' wraps all the stages into a convenient bundle and maintains global scope. </br>

To use the code, you just create an evaluator, and call 'Evaluate' with your input string.  The resulting Cell* can be printed using ToString().
Note that the Console implementation splits up the stages to make it easier to debug.

Calling the outside world is easy to do, by simply making an intrinsic function (see Intrinsics.cpp).  It's easy to embed this interpreter, but IMHO, you're 
better off with something like Lua for that ;)

The CellAllocator was also an exercise in understanding how to build a mark and sweep allocator, since I don't recall having done that before.
The algorithm simply walks out from each entry in the global scope until it has marked all cells that are reachable.  Then it 'frees' any that are left in the heap.
The allocator can either maintain a free list, or just malloc/free cells on demand.  It isn't safe to call the garbage collector during evaluation, only afterwards.
To enable that to work you have to worry about temporary cells and not collecting things that are currently in use. 

Useful Commands
---------------
(debug 1) in the REPL will let you see the memory usage, along with the symbol table.</br>
(symbols) shows all symbol table entries</br>
(variables) to see all variables at global scope.</br>
All other intrinsic functions are declared in Intrinsics.cpp, and some functions are in the startup script in SchemeInit.h

Tests
-----
Use 'git submodule init', 'git submodule update' to get the google bits for unit tests.
Tests are split into Cells, Evaluate, Tokenize and Parse, just like the evaluator.  That way it's easy to test isolated functionality.
The tests in ParseTests.cpp are Norvig's original tests.
I have a VC plugin that adds the tests to the test explorer and runs them automatically after each build; it's a neat way to write code :)

Missing stuff
-------------
Macros </br>
Complex/Hex numbers </br>
Bignums </br>
Library functions (since macro support is usually used to implement them) </br>

