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
* [Python Interface](#python)
	* [Class Inheritance Tree](#py_tree)
	* [High Level Interface](#py_high_level)
	* [Low-Level Interface](#py_low_level)
	    * [encodeData() and DecodeData()](#py_encode_data)
		* [Class Notes](#py_classes)
		    * [BoolObj](#py_boolobj)
		    * [FloatObj](#py_floatobj)
			* [ListObj](#py_listobj)

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

The data format for dictionaries looks a lot like two lists stuck together: one
for the keys and another for the associated values. Since both lists always
share the same length, however, the length gets omitted when the values list is
encoded.

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

*Implementation Note: Having a single list of key-value pairs may have improved
binary legibility, but it would also have eliminated the ability to pack bools,
which was seen as too much of a sacrifice.*

<a name="python"></a>
## Python Interface

<a name="py_tree"></a>
### Class Inheritance Tree

* `BaseException`
    * `Exception`
        * `RuntimeError`
		    * `binon.binonobj.BinOnObj.ParseErr`
            * `binon.ioutil.EndOfFile`
		* `TypeError`
		    * `binon.binonobj.BinOnObj.TypeErr`
* `binon.binonobj.BinONObj`
    * `binon.boolobj.BoolObj`
	    * `binon.boolobj.TrueObj`
	* `binon.bufferobj.BufferObj`
	* `binon.dictobj.DictObj`
	    * `binon.dictobj.SKDict`
		* `binon.dictobj.SDict`
	* `binon.floatobj.FloatObj`
	    * `binon.floatobj.Float32`
	* `binon.intobj.IntObj`
	    * `binon.intobj.UInt`
	* `binon.listobj.ListObj`
	    * `binon.listobj.SList`
	* `binon.nullobj.NullObj`
	* `binon.strobj.StrObj`
* `binon.codebyte.CodeByte`

As you can see, `BinONObj` is the root class of all the classes that handle
specific data types. Each base data type is handled by a class ending in "Obj"
in a module of the same name (except that the module name is all in lower case).
Any specialized variants of the such classes inherit from the base type's class
and can be found in the same module. These specialized classes do not end in
"Obj".

<a name="py_high_level"></a>
### High-Level Interface

The easiest way to encode/decode your data in BinON format is to call the
`BinONObj.Encode()` and `BinONObj.Decode()` class methods (from the
`binon.binonobj` module).

`BinONObj.Encode()` expects 2 or 3 arguments:
1. `value` (object): the value to encode
2. `outF` (file object): a writable binary data stream
3. `optimize` (bool, optional): auto-optimize encoding classes where
possible

What you pass in as value can be an instrinsic Python data type or container of
such types.

	BinONObj.Encode(42, myFile)
	BinONObj.Encode([1,2,3], myFile)

It may also be a BinON class-wrapped intrinsic value.

	BinONObj.Encode(UInt(42), myFile)

More on that under [Low-Level Interface](#py_low_level).

The main things to remember about `outF` is that it needs to be writable and binary.

	with open("/path/to/foo.bin", "wb") as myFile: ...

Alternatively, you could write binary data to the standard output buffer:

	myFile = sys.stdout.buffer

A third option would be to write it into a memory buffer and later call its
`getvalue()` method to retrieve the final product.

	myFile = io.BytesIO()

The `optimize` option tells `Encode()` to look closely at your data to
determine whether it can apply a specialized BinON encoding to it. For example,
if you called `BinONObj.Encode(42,myFile)`, this would ultimately call
`IntObj(42).encode(myFile)`. If you called
`BinONObj.Encode(42,myFile,optimize=True)` instead, you would get
`UInt(42).encode(myFile)` because the encoder would realize 42 is not just an
integer but an unsigned integer.

`optimize` should give you the tightest encoding in most situations, but it
comes at a cost in that it has to run over your data once before deciding what
to do. This can be particularly expensive with container types. For example, in
encoding a list of integers, it would need to check that all elements are indeed
integers before substituting an `SList` for a `ListObj`.

The alternative to using `optimize` is to be more explicit about the data
types you are encoding, as described in the low-level interface next.

<a name="py_low_level"></a>
### Low-Level Interface

The low-level interface involves dealing more directly with the classes that
handle specific data types. In particular, you may want to apply specialized
data types like `UInt` or `Float32` before encoding them. You can do so at the
scalar level (e.g. `UInt(42)`) or the container level (e.g.
`SList([1,2,3],elemCls=UInt)`).

At this point, you could directly call the `encode()` method directly on the
object you just created, or fall back on the higher level `Encode()` class
method. The latter may be a good choice if you are only applying specialized
classes to parts of your data structure. For example, say you are encoding a
data structure containing some specialized elements but not others.

	BinONObj.Encode(["foo", -1, Float32(1.0/3)], myFile)

This would give you the same output as:

	ListObj([StrObj("foo"), IntObj(-1), Float32(2.5)]).encode(myFile)

`BinONObj.Encode()` supplies base data type classes (those ending in "Obj") for
basic Python types automatically, so you only need worry about places where you
want a specialized class (one not ending in "Obj") to encode a particular value.
Here, if we had simply written `2.5` instead of `Float32(2.5)`,
`BinONObj.Encode()` would have supplied `FloatObj(2.5)` instead, which encodes
as double-precision (64 bits). (Note that in this particular case, setting
`optimize=True` in the `BinONObj.Encode()` call would give you a `Float32`
even without the explicit casting, but for floating-point in particular, it is
not a great idea to rely on this. See notes on the [floatobj](#float) module.)

<a name="py_encode_data"></a>
#### encodeData() and DecodeData()

The full encoding of a BinON object consists of a code byte indicating the data type followed by a number of bytes of object data. If the data type is obvious from the context of what you are doing, you may omit it by calling the `encodeData()` method on a BinON wrapper instance of the type in question.

For example, `IntObj(42).encode(myFile)` should output the hexadecimal: 21 2a. `IntObj(42).encodeData(myFile)` would only output: 2a.

`encodeData()` leaves object type identification in your hands, so you must later call the correct counterpart `DecodeData()` method; in this case, `IntObj.DecodeData(myFile)`. (Note that BinON follows a convention that class methods begin with a capital letter while instance methods begin in lower case.)

BinON itself uses `encodeData()` internally when, for example, it needs to store the length of a container which will always be a `UInt`.

Note that with the exception of `NullObj`, `encodeData()` will always write at
least one byte of data. (It would not make sense to use the default value mode
where the value is encoded into the code byte if the code byte does not exist!)
For example, `BoolObj.encodeData()` will write a 0x00 or 0x01 byte depending on
its value.

<a name="py_classes"></a>
#### Class Notes

What follows are some usage notes pertaining to specific data type classes.

<a name="py_boolobj"></a>
##### BoolObj

* When manually wrapping a value in a `BoolObj`, the value itself need not be a
`bool`. It could be anything that can evaluate as a boolean. For example, an
`int` evaluates as `False` for the value 0 and `True` for anything else. (When
calling the higher-level `BinONObj.Encode()`, however, an `int` will encode as
an `IntObj` or `UInt` instead, so be careful about relying on this.)
* When encoding a scalar bool, even the base `BoolObj` class will use the
`TrueObj` encoding for the `True` case. To put it another way, `BoolObj`
auto-specializes even without the `optimize=True` option in
`BinONObj.Encode()`. (It simply didn't make sense to use a 2-byte encoding for
booleans, though the decoder will recognize it if it encounters one and deal
with it appropriately.)
* When encoding a list of `BoolObj` in an `SList`, there is special logic which
packs the boolean values 8 to a byte with a big-endian bit order. (If the length
of the list is not a multiple of 8, the last byte will be zero-padded in its
least-significant bits.)

<a name="py_floatobj"></a>
##### FloatObj

* The main thing to bear in mind here is that `FloatObj` uses a
double-precision (64-bit) encoding by default, and if you want single-precision
(32-bit) encoding instead, it is best to explicitly specify the `Float32`
subtype. For a scalar, you can simply write `Float32(x)`. For containers, you
can write `SList(lst, elemCls=Float32)`, `SDict(dct, keyCls=str,
valCls=Float32)`, or whatever.

<a name="py_listobj"></a>
##### ListObj

* `ListObj` (and its subclass `SList`) implement `encodeElems()` and
`DecodeElems()` methods in addition to the usual `encodeData()` and
`DecodeData()`. `encodeElems()` is equivalent to `encodeData()` except that it
omits writing the length of the list first. You must therefore supply this
length later when you call `DecodeElems()`.
