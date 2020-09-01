# BinON

BinON is a JSON-like object notation that uses a more condensed binary format.

## Contents

* [Requirements](#requirements)
* [Data Format](#data_format)
    * [Code Byte](#code_byte)
	* [Object Types](#object_types)
	    * [Null Objects](#null)
	    * [Boolean Objects](#bool)
	    * [Integer Objects](#int)
	    * [Floating-Point Objects](#float)
		* [Buffer Objects](#buffer)
		* [String Objects](#string)
		* [List Objects](#list)
		* [Dictionary Object](#dict)

<a name="requirements"></a>
## Requirements

The complete codec has been implemented in Python 3.6. It is a self-contained repository with no dependencies outside of standard Python modules.

*(Eventually, I plan to implement a higher performance library in C++. It may
require some third party libraries to handle big integers and such, but there
should be a precompiler option to compile as much as possible with just the
standard library.)*

<a name="data_format"></a>
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

<a name="object_types"></a>
### Object Types

<a name="null"></a>
#### Null Objects

A null object has a code byte of 0 and no extra data, so even if you encode a list of a hundred nulls, nothing will get written beyond the length of the list.

Binary Encoding:

| Code Byte  | Data | Python Value | Class   |
| :--------- | :--- | -----------: | :------ |
| `00000000` |      |         None | NullObj |

<a name="bool"></a>
### Boolean Objects

Binary Encoding:

| Code Byte  | Data       | Python Value | Class     |
| :--------- | :--------- | -----------: | :-------- |
| `00010000` |            |        False | BoolObj   |
| `00010001` | `0000000b` |    bool(*b*) | BoolObj   |
| `00010010` |            |         True | TrueObj   |

Though the middle form is legal BinON, it would rarely if ever get used for a
single boolean like this, since it is better to use the default `BoolObj` value
for `False` or the `TrueObj` variant for `True`.

Note, however, that SList has a special case for encoding boolean data: it uses
the middle code byte and then packs 8 bits to the byte, rather than wasting an
entire data byte for each boolean value.

<a name="int"></a>
#### Integer Objects

The `IntObj` base type can encode signed integers of any length, but there is
also a `UInt` variant for encoding unsigned integers. In the latter case, you
can manually ensure that your numbers encode as unsigned integers by wrapping
them in `UInt` or specifying that type in a simple container class like `SList`.
Otherwise, you can let auto-specialization figure it out.

Binary Encoding:

| Code Byte  | Data                       | Python Value | Range      | Class  |
| :--------- | :------------------------- | -----------: | :--------- | :----- |
| `00100000` |                            |            0 | [0,0]      | IntObj |
| `00100001` | `0iiiiiii`                 |    int(*i…*) | [-2⁶,2⁶)   | IntObj |
| `00100001` | `10iiiiii` `iiiiiiii`      |    int(*i…*) | [-2¹³,2¹³) | IntObj |
| `00100001` | `110iiiii` `iiiiiiii`\*3   |    int(*i…*) | [-2²⁸,2²⁸) | IntObj |
| `00100001` | `1110iiii` `iiiiiiii`\*7   |    int(*i…*) | [-2⁵⁹,2⁵⁹) | IntObj |
| `00100001` | `11110000` `iiiiiiii`\*8   |    int(*i…*) | [-2⁶³,2⁶³) | IntObj |
| `00100001` | `11110001` U `iiiiiiii`\*U |    int(*i…*) | unbounded  | IntObj |
| `00100010` | `0iiiiiii`                 |    int(*i…*) | [0,2⁷)     | IntObj |
| `00100010` | `10iiiiii` `iiiiiiii`      |    int(*i…*) | [0,2¹⁴)    | IntObj |
| `00100010` | `110iiiii` `iiiiiiii`\*3   |    int(*i…*) | [0,2²⁹)    | IntObj |
| `00100010` | `1110iiii` `iiiiiiii`\*7   |    int(*i…*) | [0,2⁶⁰)    | IntObj |
| `00100010` | `11110000` `iiiiiiii`\*8   |    int(*i…*) | [0,2⁶⁴)    | IntObj |
| `00100010` | `11110001` U `iiiiiiii`\*U |    int(*i…*) | ≥ 0        | IntObj |

As you can see above, integers encode in a variable-length format. Typical values will be either 1, 2, 4, or 8 bytes in length, with the most-signficant bits of the first byte indicating the length using a scheme somewhat reminiscent of UTF-8.

For values requiring more than 64 bits, a single 0xf1 byte is followed by a
`UInt` data encoding (only the data without the code byte) of the number of
bytes needed to store the integer data that follows. This recursively encoded
byte length integer is represented by U above.

*Implementation Note: This format is something of a compromise between
legibility (in a hex editor) and compression. One could, for example, exclude
values in the 1-byte range from the 2-byte encoding to cover a somewhat larger
range in the latter, but this would come at the expense of legibility (not to
mention further complexity in the codec implementation). Likewise, in the big
int encoding, one could make the unsigned integer 9 less than the number of
bytes to follow (figuring that byte counts <= 8 would be handled in other ways),
but again, the savings are marginal at the expense of legibility.*

<a name="float"></a>
#### Floating-Point Objects

The base `FloatObj` encodes a value in double-precision (64 bits). If you only
need single-precision, it is best to wrap your values in `Float32` (or specify
the `Float32` type in a simple container class like `SList`).
(Auto-specialization is not particularly effective in the floating-point case.
It can only check if any precision is lost after encoding/decoding a value,
which means only fairly trivial values like float-encoded integers may pass the
test.)

Binary Encoding:

| Code Byte  | Data          | Python Value      | Class    |
| :--------- | :------------ | ----------------: | :------- |
| `00110000` |               |               0.0 | FloatObj |
| `00110001` | `ffffffff`\*8 |       float(*f…*) | FloatObj |
| `00110002` | `ffffffff`\*4 |       float(*f…*) | Float32  |

<a name="buffer"></a>
#### Buffer Objects

A buffer object encodes an arbitrary sequence of binary data. It writes the
length of the data (in bytes) first as a `UInt` data encoding (without the code
byte), followed the data bytes themselves.

Binary Encoding:

| Code Byte  | Data            | Python Value | Class     |
| :--------- | :-------------- | -----------: | :-------- |
| `01000000` |                 |          b'' | BufferObj |
| `01000001` | U `bbbbbbbb`\*U |  bytes(*b…*) | BufferObj |

Here, U represents the `UInt` byte count.

<a name="string"></a>
#### String Objects

A string object is really nothing more than a [buffer object](#buffer)
containing a string encoded in UTF-8.

Binary Encoding:

| Code Byte  | Data            | Python Value                | Class     |
| :--------- | :-------------- | --------------------------: | :-------- |
| `01010000` |                 |                         b'' | BufferObj |
| `01010001` | U `bbbbbbbb`\*U |  bytes(*b…*).decode('utf8') | BufferObj |

Here, U represents a `UInt` data-encoded byte count.

<a name="list"></a>
#### List Objects

All list objects begin with an element count, encoded as `UInt` data (without
the code byte). Next, in a base `ListObj`, each element is encoded in full (code
byte + any object data).

An `SList`, or simple list, is a list in which all elements share the same data
type. In this case, a single code byte identifying said type is followed by the
data for each element without its code byte.

`SList` handles boolean elements in a special way. It packs 8 to a byte to save
space.

The auto-specialization logic for lists checks all the elements in your list to
see if they share a common type and substitutes an `SList` for a `ListObj` if
that is the case. It is smart enough to select the more general data type when
necessary. For example, if all elements are unsigned integers, an `SList` of
`UInt` will be selected, but if even one of the elements is negative, the
element type will be the more general `IntObj` instead.

Binary Encoding:

| Code Byte  | Data                       | Python Value | Class   |
| :--------- | :------------------------- | -----------: | :------ |
| `10000000` |                            |           [] | ListObj |
| `10000001` | U O\*U                     |       [*O…*] | ListObj |
| `10000010` | U `cccccccc` D\*U          |       [*D…*] | SList   |
| `10000010` | U `00010001` bbbbbbbb\*U/8 |       [*b…*] | SList   |

Here:
* U is the list element count encoded as a `UInt`'s data without the code byte.
* O represents a full encoding (including the code byte) of each list element.
* The c bits represent the code byte shared by all elements in a simple list.
* D represents a data-only encoding (without the code byte) of each element.
* Each b is a single boolean value for the special case of bools which get
packed 8 to a byte with big-endian bit ordering. If U is not a multiple of 8 in
this case, the last byte will be padded out with zeros in its least-significant
bits.

<a name="dict"></a>
#### Dictionary Objects

The data format for dictionaries looks a lot like two lists stuck together: one for the keys and another for the associated values. Since both lists always share the same length, however, the length gets omitted when the values list is encoded.

Binary Encoding:

| Code Byte  | Data                              | Python Value   | Class   |
| :--------- | :-------------------------------- | -------------: | :------ |
| `10010000` |                                   |             {} | DictObj |
| `10010001` | U K\*U V\*U                       |  {*K*:*V* *…*} | DictObj |
| `10010010` | U `cccccccc` k\*U V\*U            |  {*k*:*V* *…*} | SKDict  |
| `10010011` | U `cccccccc` k\*U `dddddddd` v\*U |  {*k*:*v* *…*} | SDict   |

Here:
* U is the list element count encoded as a `UInt`'s data without the code byte.
* K represents a full encoding (including the code byte) of each dictionary key.
* V represents a full encoding (including the code byte) of each dictionary
value.
* The c bits represent the code byte shared by all dictionary keys.
* The d bits represent the code byte shared by all dictionary values.
* Lower case k is like K except the code bytes for each key are omitted.
* Lower case v is like V except the code bytes for each value are omitted.

Note that in simple dictionaries, boolean values get packed 8 to a byte as they
do in simple lists.

*Implementation Note: Having a single list of key-value pairs may have improved binary legibility, but it would also have eliminated the ability to pack bools, which was seen as too much of a sacrifice.*