# BinON

BinON is a JSON-like object notation that uses a more condensed binary format.

## Requirements

The complete codec has been implemented in Python 3.6. It is a self-contained repository with no dependencies outside of standard Python modules.

(Eventually, I plan to implement a higher performance library in C++. It may
require some third party libraries to handle big integers and such, but there
should be a precompiler option to compile as much as possible with just the
standard library.)

## Data Format

A fully-encoded BinON object consists of a [code byte](#code_byte) followed by
zero or more bytes of object data. The code byte essentially identifies the data
type while the data encode the value itself. Where the data include multi-byte
scalars, BinON follows a big-endian byte ordering convention.

In some cases, there may be no data following the code byte. The [null](#null)
data type would be an obvious example, but also, every data type includes a
default value which can be encoded into the code byte itself. In Python terms,
the default value is the one that evaluates as `False` in a conditional
expression. For example, the default `int` would be `0`.

Conversely, you can encode just the object data without the code byte if the
data type is obvious from context. For example, BinON uses unsigned integers
within container types to indicate the size of the container, but there is no
need to include the code byte indicating that it's an integer in that case.

<a name="code_byte"></a>
### Code Byte

The code byte is composed of two 4-bit fields:

| Bits | Value        |
| ---: | :----------- |
| 8→4  | base type ID |
| 3→0  | subtype ID   |

Base types represent complete data types that can be encoded directly, using a
subtype ID of 0 or 1. 1 indicates that base type data are to follow, while 0
indicates we are looking at the default value with no extra data to encode.

Subtypes of 2 or higher indicate variants on the base type. For example,
unsigned integers are a variant of general integers.

| Base Type | Subtype | Class       | Python Type  | Value   |
| --------: | ------: | :---------- | :----------- | ------: |
|         0 |       0 | `NullObj`   | `type(None)` |  `None` |
|         1 |       0 | `BoolObj`   | `bool`       | `False` |
|         1 |       1 | `BoolObj`   | `bool`       | follows |
|         1 |       2 | `TrueObj`   | `bool`       |  `True` |
|         2 |       0 | `IntObj`    | `int`        |     `0` |
|         2 |       1 | `IntObj`    | `int`        | follows |
|         2 |       2 | `UInt`      | `int`        | follows |
|         3 |       0 | `FloatObj`  | `float`      |   `0.0` |
|         3 |       1 | `FloatObj`  | `float`      | follows |
|         3 |       2 | `Float32`   | `float`      | follows |
|         4 |       0 | `BufferObj` | `bytes`†     |   `b''` |
|         4 |       1 | `BufferObj` | `bytes`      | follows |
|         5 |       0 | `StrObj`    | `str`        |    `''` |
|         5 |       1 | `StrObj`    | `str`        | follows |
|         8 |       0 | `ListObj`   | `list`†      |    `[]` |
|         8 |       1 | `ListObj`   | `list`       | follows |
|         8 |       2 | `SList`     | `list`       | follows |
|         9 |       0 | `DictObj`   | `dict`       |    `{}` |
|         9 |       1 | `DictObj`   | `dict`       | follows |
|         9 |       2 | `SKDict`    | `dict`       | follows |
|         9 |       3 | `SDict`     | `dict`       | follows |

† Note that the Python types listed above pertain to the type that gets decoded.
On encoding, `BufferObj` will accept any bytes-like type including `bytes` or
`bytearray`. `ListObj/SList` will accept any iterable type that implements
`__len__()`; so `list`, `tuple`, and even `set` are fair game.

### Object Types

Scalar types:
* [null](#null)
* [boolean](#bool)
* [integer](#int)

<a name="null"></a>
### Null Objects

Full Binary Encoding:

| Code Byte  | Data | Python Value |
| :--------- | :--- | -----------: |
| `00000000` |      |       `None` |

A null object has a code byte of 0 and no extra data, so even if you encode a list of a hundred nulls, nothing will get written beyond the length of the list.

<a name="bool"></a>
### Boolean Objects

Full Binary Encoding:

| Code Byte  | Data       | Python Value |
| :--------- | :--------- | -----------: |
| `00010000` |            |      `False` |
| `00010001` | `0000000b` |    `bool(b)` |
| `00010010` |            |       `True` |

Though the middle form is legal BinON, it would rarely if ever get used, since
it is better to encode the true/false state within the code byte.

Note that SList has a special case for encoding boolean data: it packs 8 bits to
the byte, rather than wasting an entire data byte for each value.

<a name="int"></a>
### Integer Objects

Full Binary Encoding:

| Code Byte  | Data                      | Python Value      |
| :--------- | :------------------------ | ----------------: |
| `00100000` |                           |                 0 |
| `00100001` | `0iiiiiii`                |   -2^6 <= i < 2^6 |
| `00100001` | `10iiiiii` `iiiiiiii`     | -2^13 <= i < 2^13 |
| `00100001` | `110iiiii` `iiiiiiii`*3   | -2^28 <= i < 2^28 |
| `00100001` | `1110iiii` `iiiiiiii`*7   | -2^59 <= i < 2^59 |
| `00100001` | `11110000` `iiiiiiii`*8   | -2^63 <= i < 2^63 |
| `00100001` | `11110001` U `iiiiiiii`*U |           any int |
| `00100010` | `0iiiiiii`                |      0 <= i < 2^7 |
| `00100010` | `10iiiiii` `iiiiiiii`     |     0 <= i < 2^14 |
| `00100010` | `110iiiii` `iiiiiiii`*3   |     0 <= i < 2^29 |
| `00100010` | `1110iiii` `iiiiiiii`*7   |     0 <= i < 2^60 |
| `00100010` | `11110000` `iiiiiiii`*8   |     0 <= i < 2^64 |
| `00100010` | `11110001` U `iiiiiiii`*U |            i >= 0 |

Integer objects use a variable-length encoding format. The base type (`IntObj`)
can encode signed integers of any length, but there is also a variant (`UInt`)
for encoding unsigned integers.

For typical integers, the encoding somewhat resembles UTF-8 using 1, 2, 4, and
8-byte quantities. For values approaching the 64-bit limit, there is a 9-byte
encoding: a single 0xf0 byte, followed by the 64-bit value in 8 bytes.

For values requiring more than 64 bits, a single 0xf1 byte is followed by a UInt
data encoding of the number of bytes needed to store the integer data that
follows. This recursively encoded integer is represented by U above.