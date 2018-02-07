## TODO
- parsing: synchronize
- logger
    - rename error.c to log.c? or add log.c?
    - move `info` into `report` and generalize
        - pass a level and location to report()
        - update `info` and `error` to use `report` properly
    - no ansi stuff if `!istty`
- rename `scanner.c` to `lexer.c`?
- better memory management

