binon
=====

Implements a JSON-like object notation in a typically more condensed
binary format.

Multi-byte elements always follow a big-endian byte ordering,
where a "byte" must be exactly 8 bits.

The format is currently supported in 2 languages:

1. Python 3 or later (py3 directory).
2. C++17 or later (cpp17 directory).

Python currently implements the entirety of the data format,
while C++17 is restricted to a subset.

Data Types
__________

The first byte in any value encoding signifies the data type.
Here is a list of type IDs and what they represent.
For example, a 0x03 byte would represent an integer.

0. null
1. false
2. true
3. [integer][int] (C++ up to 64-bit signed/unsigned, Python unlimited)
4. [floating-point number][float]
5. [byte buffer][bytes]
6. [string][str]
7. [simple list][slist]
8. [general list][list]
9. [simple dictionary][sdict]
10. [simple key dictionary][skdict]
11. [general dictionary][dict] (Python only)

Type Data Formats
_________________

### null, false, true ###

These are the only types requiring only a single byte to encode.
In Python, they correspond to None, False, and True,
encoding as 0x00, 0x01, and 0x02, respectively.

### [int]:integer ###

Integers have the most complex data format in binon.
After the initial 0x03 byte signifying integer, they use a
variable-length encoding somewhat reminiscent of UTF-8.

	| From  | To     | Bits                |
	| ----: | -----: | :------------------ |
	|  -2^6 |  2^6-1 | 0sssssss            |
	| -2^13 | 2^13-1 | 10ssssss ssssssss   |
	| -2^28 | 2^28-1 | 110sssss ssssssss*3 |
	| -2^59 | 2^59-1 | 1110ssss ssssssss*7 |
	| -2^63 | 2^63-1 | 11111100 ssssssss*8 |
	|     0 | 2^64-1 | 11111101 uuuuuuuu*8 |

Here, the s bits signify the integer's value in two's complement form.
There is clearly some overlap in these ranges, and technically, there is
no reason you could not encode the value 1 in 9 bytes. But the encoding
algorithm implemented here will always work down the above table
and go with the first option that can handle the value.

You can see in the last 2 cases that the first byte is a fixed code
not containing any s data bits. In that byte, all but the least-significant
2 bits are set. The 2 bits are flags, with the lower one indicating whether
the integer should be considered unsigned.

The table shows all the possible integer encodings for C++, but Python 3
can handle big integers > 64 bits. For these, we switch to an arbitrary length
encoding like this:

	11111110 <INT> ssssssss*(<INT>+9)
	11111111 <INT> uuuuuuuu*(<INT>+9)

The 2nd-least-significant bit in the code byte signals big integer mode.
After the code byte comes a recursively-encoded integer indicating how many
bytes of actual integer data are to follow less 9 (since we wouldn't need
need this encoding for <= 8 bytes). (Note that <INT> here does not include
the 0x03 byte specifying that it is an int, since that is rather implied.)
For example, the value 2^128-1 would look like this in hex:

	03FF07FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFF

So we have 0x03 = int, 0xFF = unsigned big int, 0x07 + 9 = 16 data bytes,
and finally the 16 0xFF bytes themselves.

### [float]:floating-point number ###

This is the code 0x04 followed by a length byte and an IEEE 754-encoded
floating-point value. Currently, 32- and 64-bit floats are supported.
For example, a 32-bit 1.0 would encode as hex:

	043F8000 00

### [bytes]:byte buffer ###

After the initial 0x05 byte, we encode an integer N followed by N data
bytes from the buffer. The integer uses out integer encoding format
(without the 0x03 byte).

Example:

	0504DEAD BEEF

### [str]:string ###

This is really nothing more than a [byte buffer][bytes] containing a
UTF8-encoded string. In Python 3, this will decode to a str (rather than
bytes) object. (Note that the encoded length is the number of bytes
after converting to UTF8 and *not* the number of unicode code points.)

### [slist]:simple list ###

A simple list is a container in which all elements are of the same type.
Compared to a general list, it can encode smaller because it does not
need to store type info for every element. The encoding looks like this:

	0x07 0xTT <INT> <ELEM>*<INT>

The 0x07 indicates this is a simple list. The 0xTT byte contains the
data type code for all the elements. This is followed by an integer
specifying the number of elements, and finally the elements themselves
encoding without their preceeding type codes.

Say you wrote a simple list of 4 integers: 2, 1, 0, -1
In hex, you would get:

	07040201 003F

### [list]:general list ###

In a general list, the elements can have different types and encode
like this:

	0x08 <INT> (0xTT <ELEM>)*<INT>

If you had a list containing integer 1 and 32-bit float 1.0, you would get:

	08020301 04043F80 0000

### [sdict]:simple dictionary ###

A dictionary (a.k.a. a hash table) is an unordered associative container
matching keys to values. In a simple dictionary, the keys all need to be
of the same data type and the values also need to be of the same data type
(which may be different from that of the keys).

The encoding looks like this:

	0x09 0xKK 0xVV <INT> (<KEY> <VAL>)*<INT>

0xKK is the data type code for all keys
0xVV is the data type code for all values
<INT> is the number of elements in the dictionary (encoding using our
variable length integer format).
<KEY> is each key encoded without its type ID
<VAL> is each corresponding value encoded without its type ID

### [skdict]:simple key dictionary ###

This is like a [simple dictionary][sdict] except the values can differ in
data type. The encoding changes to this:

	0x0A 0xKK <INT> (<KEY> 0xVV <VAL>)*<INT>

### [dict]: dictionary ###

In this most-general form of a dictionary, both the keys and values have
variable data types. This really only works in Python and encodes to:

	0x0B <INT> (0xKK <KEY> 0xVV <VAL>)*<INT>

(See [simple dictionary][sdict] for an explanation of how to read this.)
