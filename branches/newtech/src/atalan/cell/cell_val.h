/*

Value cell represents some constant value of specific type.

It is useful in rules, where we need to differentiate between variables and values before translating them to CPU instructions.

type    represents a type, that the value must fit. It is typically a CELL_RANGE.


+-----------------+
| CELL_VAL        |
+-----------------+    +-----------------+
| type            |--->|  CELL_*         |
+-----------------+    +-----------------+


Value is mostly used when matching using CELL_MATCH.

+-----------------+
| CELL_MATCH      |
+-----------------+
| l (arg)         |
+-----------------+    +-----------------+
| r               |--->|  CELL_VAL       |
+-----------------+    +-----------------+    +-----------------+
                       | type            |--->|  CELL_*         |
                       +-----------------+    +-----------------+


*/
