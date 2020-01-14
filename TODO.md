# TODO for Boombase #

## Change backend to play atomic ##

- Make changes to movegen for atomic
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

- Write unit tests for movegen.h (lookup table checks) (?)


## Less urgent (nice-to-have) ##

- Refactor Position to be more modular (turn Position into an interface + data and common logic, and delegate the business logic of makeMove() and unmakeMove() to specialised MoveMaker? classes. (Strategy? Command?))
    - i.e. OrthoMoveMaker implements MoveMaker, AtomicMoveMaker implements MoveMaker, where MoveMaker is simply an interface of two functions, makeMove() and unmakeMove().
    - Why? This would make it easier to add more variant support at a much later stage.
    - Hence why it's not an urgent update...

- Improve test suites (lower priority)
    - Negative tests (ones expected to fail) can be given?
    - Output can be prettier
    - More details can be given (summary of failed tests, output and expected output)