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

	11111110 N=iiiiiiii*M ssssssss*(N+9)
	11111111 N=iiiiiiii*M uuuuuuuu*(N+9)

Here, the `M` bytes of `i` bits represent the integer `N`, encoded by calling
the integer encoder recursively. `N` is the number of bytes of data to follow
less 9 (since there would be no point in using a big integer representation if
we didn't have > 8 bytes to follow). (Note that the `i` bits do *not* include
a 0x03 type ID, as it is self-evident from the context that we are dealing with
an integer.)

For example, here is a full encoding (including the type ID) of the value
2^128-1 in hexadecimal:

	03FF07FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFF

That's 3 bytes of metadata followed by 16 bytes of integer data.

<a name="float"></a>
### float (id=0x04) ###

Floating-point numbers have an bit-level object data representation like this:

	00000100 ffffffff*4
	00001000 ffffffff*8

where the `f` bits represent an IEEE 754-encoded float.

In other words, the first byte (following the 0x04 type ID) gives the length of
the float data in bytes: 4 or 8 for 32- or 64-bit floats, respectively.
(At this point, those are the only 2 lengths supported.)

<a name="bytes"></a>
### byte buffer (id=0x10) ###

A byte buffer is a variable-length sequence of bytes with a bit-level
object data representation like this:

	N=iiiiiiii*M dddddddd*N

Here, the `M` bytes of `i` bits represent an integer `N`, encoded using
the [integer](#int) format (without the 0x03 type ID).

Following this are the `N` bytes of buffer data.

<a name="str"></a>
### string (id=0x11) ###

Strings are essentially just [byte buffers](#bytes) of UTF-8-encoded text,
differing only in their type ID.

<a name="slist"></a>
### simple list (id=0x20) ###

A simple list is a vector in which all values share the same type.
The type ID, therefore, need only be encoded once.

The most data types, the object data take the bit-level form:

	N=iiiiiiii*M tttttttt (dddddddd*T)*N

where:

- The `i` bits encode the integer `N` (see [byte buffer](#bytes)).
- The `t` bits encode the data type ID of the list elements.
- The `d` bits represent `T` bytes of data for each element.

Note that with simple lists, there are special cases for dealing with data types
in which the value is typically encoded into the type ID itself, which would not
make sense here.

- For a list of **boolean**, the `t` bits may encode either 0x01 (false) or
0x02 (true), while the `d` bits in this case would be the boolean values
packed 8 to a byte with zero-padding in the least-signficant bits of
the final byte if necessary (in keeping with the big-endian formatting
principle).
- For a list of **null**, the `t` bits would encode 0x00 as you would expect,
but there would be no `d` bits following these.

<a name="list"></a>
### general list (id=0x21) ###

In a general list, each element can be of a different data type, so compared
to a [simple list](#slist), the object data take the form:

	N=iiiiiiii*M (tttttttt dddddddd*T)*N

In other words, we are encoding `N` type-value pairs.

<a name="sdict"></a>
### simple dictionary (id=0x30) ###

Dictionaries are essentially encoded as two lists: one for the keys and one for
the corresponding values. But since both lists are the same length, the integer
indicating this length need only be encoded once at the beginning of the
object data.

In a simple dictionary, all keys share the same data type. All values also
share the same data type (though it may differ from that of the keys).

This means the object data are nothing more than two [simple lists](#slist)
sharing a length:
	
	N=iiiiiiii*M kkkkkkkk (cccccccc*K)*N vvvvvvvv (dddddddd*V)*N

where:

- The `i` bits encode the integer N (see [byte buffer](#bytes)).
- The `k` bits are the key data type ID.
- The `v` bits are the value data type ID.
- The `c` bits represent `K` bytes of data for each key.
- The `d` bits represent `V` bytes of corresponding value data.

See also [simple list](#slist) for notes on the special cases of how
boolean and null values get encoded.

<a name="skdict"></a>
### simple key dictionary (id=0x31) ###

A simple key dictionary differs from a [simple dictionary](#sdict) in that
while the keys must all share the same data type, the values may differ from
one another in data type.

Rather than combining two simple lists, then, we need a simple list and a
general list.

	N=iiiiiiii*M kkkkkkkk (cccccccc*K)*N (vvvvvvvv dddddddd*V)*N

<a name="skdict"></a>
### general dictionary (id=0x32) ###

In a general dictionary, both the keys and the values may have varying data
types. The C++ implementation does not currently support this type of
dictionary.

The object data combine two [general lists](#list) sharing a length:

	N=iiiiiiii*M (kkkkkkkk cccccccc*K)*N (vvvvvvvv dddddddd*V)*N
