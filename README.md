# 8051-small-calculator

## features

- Basic operations:

| Operation      | C Operator  | Display on 7-Segment LED |
|----------------|-------------|--------------------------|
| Addition       | +           | ┤                        |
| Subtraction    | -           | ─                        |
| Multiplication | *           | ├┤                       |
| Int-division   | //          | ┌┘                       |
| Float-division | /           | ┌┘.                      |
| Modulo         | %           | ┌┐                       |

- Support both Dec-to-Hex and Hex-to-Dec conversion.
- Support 32-bit integer calculations.
- Support float calculations (range: 0.65535 to 65535, only in Dec).
- Support negative values for both int and float (Hex supports complement).
- Support calculation history for the last 10 records.
- The operator will show on the left of the LED while selecting an operator.
- An `h` or `d` will show on the left of the LED while not selecting an operator, indicating hex or dec mode.

## button UI

- The calculator has 16 buttons (4x4 grid).

### Dec mode

|   |   |   |   |
|---|---|---|---|
| 7 | 8 | 9 | del|
| 4 | 5 | 6 | op|
| 1 | 2 | 3 | = |
| 0 | - | < | h |

- The digit buttons are simply `[0]`~`[9]`.
- `[del]` button removes the last digit pressed, does nothing if the value is 0.
- `[op]` button cycles through operations `[+]`, `[-]`, `[*]`, `[//]`, `[/]`, `[%]`, the operation applies after pressing a new digit.
- `[-]` button switches between negative and positive.
- `[<]` button shifts the display number, as the 8051 has only 8 7-segment LEDs to show at most 8 digits.
- `[=]` gets the result; pressing it multiple times loops through the history of results.
- `[h]` switches to Hex mode.

### Hex mode

|   |   |   |   |
|---|---|---|---|
| 7 | 8 | 9 | del|
| 4 | 5 | 6 | op|
| 1 | 2 | 3 | = |
| 0 | ^ | < | d |

- `[^]` switches to the additional keyboard, allowing access to `[A]`~`[F]` buttons.
- `[d]` switches to Dec mode.

### Hex mode (addition keyboard)

|   |   |   |   |
|---|---|---|---|
|   |   |   | del|
| E | F |   | op|
| B | C | D | = |
| A | ^ | < | d |

- The `[A]`~`[F]` buttons are simply digits 10~15 for Hexadecimal.
- `[^]` switches back to normal keyboard.
