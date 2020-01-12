# TODO for Boombase #

## Change backend to play atomic ##

- Make changes to movegen and Position for atomic
    - Change StateInfo and undoStack to work by pointers to try and salvage ortho/atomic switchability
- Test with perft against FOX's perft (in shakmaty's github repo, the Rust chess engine).
- Implement checkmate and stalemate checking
- Implement insufficient material checking
- Implement Zobrist hashing and repetition checking


## Decide on implementation for database/search/files ##

- Choose what PGN headers to retain.
- Choose what information is in Game.h
- Define file structure (how many? what fields?)


## Test out Qt ##

- Make a helloworld in Qt, and play with the GUI tools.
- Optional: Compare Qt and Tcl/Tk.


## Miscellaneous improvements ##

- Write unit tests for movegen.h (lookup table checks)


## Less urgent (nice-to-have) ##

- Improve test suites (lower priority)
    - Chronometry (time the tests)
    - Negative tests (ones expected to fail) can be given?
    - Output can be prettier
    - More details can be given (summary of failed tests, output and expected output)