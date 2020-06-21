BinON
=====

BinON is a JSON-like object notation that uses a more condensed binary format.

This is a self-contained repository implementing a complete codec in Python 3,
as well as a somewhat limited (but presumably faster) implementation
in C++17.

Data Format
-----------

At a basic level, the data format consists of a single byte type ID followed by
zero or more bytes of object data. Here, a "byte" is defined as 8 bits.

Multi-byte values within the object data use a big-endian byte ordering.

Data Type IDs
-------------

BinON supports 12 data types, each with a unique ID.

### Scalars ###

0. [null](#null)
1. [false](#null)
2. [true](#null)
3. [integer](#int) (up to 64-bit in C++, unlimited in Python)
4. [float](#float)

### Buffers ###

16. [byte buffer](#bytes)
17. [string](#str)

### Vector Containers ###

32. [simple list](#slist)
33. [general list](#list)

### Associative Containers ###

48. [simple dictionary](#sdict)
49. [simple key dictionary](#skdict)
50. [general dictionary](#dict) (Python only)

Data Type Encoding
------------------

<a name="null"></a>
### null (id=0x00), false (id=0x01), true (id=0x02) ###

These 3 data types are entirely expressed by their type IDs alone.
Python's `None`, `False`, and `True` values encode as 0x00, 0x01, and 0x02,
respectively.

<a name="int"></a>
### integer (id=0x03) ###

Integers use a variable-length encoding somewhat reminiscent of UTF-8.
After the initial 0x03 type ID code, the number of bytes of object data depends
on what range the integer lies within.

| From  | To     | Bit Representation    |
| ----: | -----: | :-------------------- |
|  -2^6 |  2^6-1 | `0sssssss`            |
| -2^13 | 2^13-1 | `10ssssss ssssssss`   |
| -2^28 | 2^28-1 | `110sssss ssssssss*3` |
| -2^59 | 2^59-1 | `1110ssss ssssssss*7` |
| -2^63 | 2^63-1 | `11111100 ssssssss*8` |
|     0 | 2^64-1 | `11111101 uuuuuuuu*8` |

Here, the `s` bits signify the signed integer value in two's complement form.
For example, -1000 (0xfffffc18 as a 32-bit int) would encode to 0xbc18.
To decode, you would need to sign-extend from bit 13 past the `10` bits.

The last 2 entries in the table deal with cases approaching the limit of 2^64.
At this point, the first byte turns into a fixed code with no `s` data bits
in it. The least-significant 2 bits within the byte are treated as flags.
Bit 0 indicates whether the integer is unsigned.

Bit 1 puts the decoder in **big integer mode**:

	0b11111110 iiiiiiii*M ssssssss*(N+9)
	0b11111111 iiiiiiii*M uuuuuuuu*(N+9)

The `i` bits represent the integer N, encoded by calling the integer encoder
recursively. N is the number of bytes of data to follow less 9 (since there
would be no point in using a big integer representation if we didn't have > 8
bytes to follow). (Note that the `i` bits do *not* include the 0x03 type ID,
as it is self-evident from the context that we are dealing with an integer.)

For example, here is a full encoding (including the type ID) of the value
2^128-1 in hexadecimal:

	03FF07FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFF

That's 3 bytes of metadata followed by 16 bytes of integer.

<a name="float"></a>
### float (id=0x04) ###

Floating-point numbers have an object data representation like this:

	0b00000100 ffffffff*4
	0b00001000 ffffffff*8

In other words, the first byte (following the 0x04 type ID) is the length of
the number in bytes: 4 or 8 for 32- or 64-bit floats, respectively.
(At this point, those are the only 2 lengths supported.)
The `f` bits are the floating-point value in IEEE 754 format.

<a name="bytes"></a>
### byte buffer (id=0x10) ###

A byte buffer is a variable-length sequence of bytes whose object data looks
like this:

	0biiiiiiii*M dddddddd*N

Here, the `M` bytes of `i` bits represent an integer `N`, and the `d` bits are
`N` bytes worth of data. `N` is encoded using the [integer](#int) format
(without the 0x03 type ID).

<a name="str"></a>
### string (id=0x11) ###

A string is essentially just a [byte buffer](#bytes) with text encoded in UTF-8.
(The encoded length, therefore, is simply the number of bytes rather than the
number of code points.)

<a name="slist"></a>
### simple list (id=0x20) ###

A simple list is a vector in which all values share the same type.
The type ID, therefore, need only be encoded once.

The object data takes the form:

	0btttttttt iiiiiiii*M (dddddddd*T)*N

where:

- The `t` bits indicate the data type ID of list elements.
- The `i` bits encode the integer N (see [byte buffer](#bytes)).
- The `d` bits represent T bytes of data for each element.

<a name="list"></a>
### general list (id=0x21) ###

In a general list, each element can be of a different data type.

In this case, the object data must place this type together with each element.

	0biiiiiiii*M (tttttttt dddddddd*T)*N

See [simple list](#slist) for an explanation of what `i`, `t`, etc. mean.

<a name="sdict"></a>
### simple dictionary (id=0x30) ###

In a simple dictionary, all the keys must be of the same data type and
all the values must also be of the same data type.
(The value type need not match the key type, however.)
The object data look like this:

	0bkkkkkkkk vvvvvvvv iiiiiiii*M (cccccccc*K dddddddd*V)*N

where:

- The `k` bits are the key data type ID.
- The `v` bits are the value data type ID.
- The `i` bits encode the integer N (see [byte buffer](#bytes)).
- The `c` bits represent `K` bytes of data for each key.
- The `d` bits represent `V` bytes of the corresponding value.

<a name="skdict"></a>
### simple key dictionary (id=0x31) ###

In simple key dictionaries, all the keys must be of the same data type,
but the values may differ in data type.
In this case, the object data take the form:

	0bkkkkkkkk iiiiiiii*M (cccccccc*K vvvvvvvv dddddddd*V)*N

See [simple dictionary](#sdict) for info on what these symbols mean.

<a name="skdict"></a>
### general dictionary (id=0x32) ###

The most flexible of dictionary allows keys to have differing data types
as well as values. This data type, however, is not supported by C++ at
this time.

The object data take the form:

	0biiiiiiii*M (kkkkkkkk cccccccc*K vvvvvvvv dddddddd*V)*N

See [simple dictionary](#sdict) for info on what these symbols mean.
