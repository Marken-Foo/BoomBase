# TODO for Boombase #

## Change backend to play atomic ##

- Implement valid move checking
- Improve legal move checking and generation
- Implement checkmate and stalemate checking
- Implement insufficient material checking
- Implement Zobrist hashing and repetition checking


## Decide on implementation for database/search/files ##

- Choose what PGN headers to retain.
- Choose what information is in Game.h
- Define file structure (how many? what fields?)


## Implement PGN parser ##

- PGN traversal while creating Game object
    - Methods to use Visitor pattern (for flexibility in parsing PGN/Game objects)
    - To be done only once legal move checking has been sped up (validate, then generate only possible legal matches), and Game structure has been decided.


## Test out Qt ##

- Make a helloworld in Qt, and play with the GUI tools.
- Optional: Compare Qt and Tcl/Tk.


## Less urgent (nice-to-have) ##

- Improve test suites
    - Negative tests (ones expected to fail) can be given?
    - Output can be prettier
    - More details can be given (summary of failed tests, output and expected output)