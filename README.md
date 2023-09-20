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
		* [BinONObj.Encode()](#py_encode)
		* [BinONObj.Decode()](#py_decode)
	* [Low-Level Interface](#py_low_level)
	    * [encodeData() and DecodeData()](#py_encode_data)
* [C++ Interface](#cpp)
	* [Building Library](#cpp_build)
	* [Data Structures](#cpp_structs)
		* [BinONObj and Object Helpers](#cpp_binonobj)
		* [BufferObj and BufferVal](#cpp_bufferobj)
		* [DictObj, SKDict, SDict, and Helper Functions](#cpp_dictobj)
		* [IntObj, UIntObj, IntVal, and UIntVal](#cpp_intobj)
		* [ListObj, SList and Helper Functions](#cpp_listobj)
		* [StrObj and HyStr](#cpp_strobj)

<a name="requirements"></a>
## Requirements

The complete codec has been implemented in Python 3.6.
It is a self-contained repository with no dependencies outside of standard Python modules.

The C++ implementation requires C++17 or later, but has some optimizatins for C++20 where available. It has been compiled under versions of clang++ and g++ under macOS and Ubuntu, though the code should be relatively portable to other compilers.

<a name="data_format"></a>
## Data Format

A fully-encoded BinON object consists of an 8-bit [code byte](#code_byte) followed by zero or more additional bytes of object data. The code byte essentially identifies the data type, while the data bytes encode the value itself. Where the data include multi-byte scalars, BinON follows a big-endian byte ordering convention.

In some cases, there may be no data following the code byte. The [null](#null) data type would be an obvious example. But additionally, every data type includes a default value which can be encoded into the code byte itself. In Python terms, the default value is the one that evaluates to `False` in a conditional expression. For example, the default `int` would be `0`, and the default list would be `[]` (an empty list).

In other cases, you may choose to encode only the object data without the code byte if the data type is known from context. For example, BinON stores the number of elements in a list object. Since this will always be an unsigned integer, there is no need to include a code byte indicating so.

<a name="code_byte"></a>
### Code Byte

The code byte is composed of two 4-bit fields:

| Bits | Value        |
| ---: | :----------- |
| 8→4  | base type ID |
| 3→0  | subtype ID   |

Base types represent complete data types that can be encoded directly, using a subtype ID of 0 or 1. 1 indicates that base type data are to follow, while 0 indicates we are looking at the default value with no extra data to encode.

Subtypes of 2 or higher indicate specialized variants on the base type. They can typically encode only a subset of what the base type can handle, but may be more optimized in terms of encoding size. For example, unsigned integers are a variant of the base (signed) integers.

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

† Note that the Python types listed above pertain to the type that gets decoded. On encoding, `BufferObj` will accept any bytes-like type including `bytes` or `bytearray`. `ListObj/SList` will accept any iterable type that implements `__len__()`; so `list`, `tuple`, and even `set` are fair game.

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

Though the middle form is legal BinON from a decoding standpoint, it would rarely if ever get used, since it is better to encode the default `BoolObj` value for `False` or the `TrueObj` subtype for `True`.

SList (a ListObj subtype for handling lists in which all elements share the same data type) deals with booleans in a special way. It packs them 8 to a byte. If there are less than 8 bits for the final byte, it pads the least-significant bits with zeros (following the big-endian convention at the bit level).

<a name="int"></a>
#### Integer Objects

The `IntObj` base type can encode signed integers of any length, but there is also a `UInt` subtype for encoding unsigned integers. In the latter case, you can manually ensure that your numbers encode unsigned by wrapping them in `UInt` or specifying that type in a simple container class like `SList`. Otherwise, you can let encoding optimization logic figure it out.

Binary Encoding:

| Code Byte  | Data                       | Python Value | Range        | Class  |
| :--------- | :------------------------- | -----------: | :----------- | :----- |
| `00100000` |                            |            0 | [0,0]        | IntObj |
| `00100001` | `0iiiiiii`                 |    int(*i…*) | [-2⁶,2⁶-1]   | IntObj |
| `00100001` | `10iiiiii` `iiiiiiii`      |    int(*i…*) | [-2¹³,2¹³-1] | IntObj |
| `00100001` | `110iiiii` `iiiiiiii`\*3   |    int(*i…*) | [-2²⁸,2²⁸-1] | IntObj |
| `00100001` | `1110iiii` `iiiiiiii`\*7   |    int(*i…*) | [-2⁵⁹,2⁵⁹-1] | IntObj |
| `00100001` | `11110000` `iiiiiiii`\*8   |    int(*i…*) | [-2⁶³,2⁶³-1] | IntObj |
| `00100001` | `11110001` U `iiiiiiii`\*U |    int(*i…*) | unbounded    | IntObj |
| `00100010` | `0iiiiiii`                 |    int(*i…*) | [0,2⁷-1]     | UInt   |
| `00100010` | `10iiiiii` `iiiiiiii`      |    int(*i…*) | [0,2¹⁴-1]    | UInt   |
| `00100010` | `110iiiii` `iiiiiiii`\*3   |    int(*i…*) | [0,2²⁹-1]    | UInt   |
| `00100010` | `1110iiii` `iiiiiiii`\*7   |    int(*i…*) | [0,2⁶⁰-1]    | UInt   |
| `00100010` | `11110000` `iiiiiiii`\*8   |    int(*i…*) | [0,2⁶⁴-1]    | UInt   |
| `00100010` | `11110001` U `iiiiiiii`\*U |    int(*i…*) | ≥ 0          | UInt   |

As you can see above, integers encode in a variable-length format. Typical values will be either 1, 2, 4, or 8 bytes in length, with the most-signficant bits of the first byte indicating the length using a scheme somewhat reminiscent of UTF-8.

For values requiring more than 64 bits, a single 0xf1 byte is followed by a `UInt` data encoding (only the data without the code byte) of the number of bytes needed to store the integer data that follows. This recursively encoded byte length integer is represented by U above.

*Implementation Note: This format is somewhat of a compromise between legibility (in a hex editor) and compression. One could, for example, exclude values in the 1-byte range from the 2-byte encoding to cover a somewhat larger range in the latter, but this would come at the expense of legibility (not to mention further complexity in the codec implementation). Likewise, in the big int encoding, one could make the unsigned integer 9 less than the number of bytes to follow (figuring that byte counts ≤ 8 would be handled in other ways), but again, the savings would be marginal at the expense of legibility.*

<a name="float"></a>
#### Floating-Point Objects

The base `FloatObj` encodes a value in double-precision (64 bits). If you only need single-precision, it is best to wrap your values in `Float32` (or specify the `Float32` type in a simple container class like `SList`). (Auto-optimization is not particularly effective in the floating-point case. It can only check if any precision is lost after encoding/decoding a value, which means only fairly trivial values like float-encoded integers may pass the test.)

Binary Encoding:

| Code Byte  | Data          | Python Value      | Class    |
| :--------- | :------------ | ----------------: | :------- |
| `00110000` |               |               0.0 | FloatObj |
| `00110001` | `ffffffff`\*8 |       float(*f…*) | FloatObj |
| `00110002` | `ffffffff`\*4 |       float(*f…*) | Float32  |

<a name="buffer"></a>
#### Buffer Objects

A buffer object encodes an arbitrary sequence of binary data. It writes the length of the data (in bytes) first as a `UInt` data encoding (without the code byte), followed the data bytes themselves.

Binary Encoding:

| Code Byte  | Data            | Python Value | Class     |
| :--------- | :-------------- | -----------: | :-------- |
| `01000000` |                 |          b'' | BufferObj |
| `01000001` | U `bbbbbbbb`\*U |  bytes(*b…*) | BufferObj |

Here, U represents the `UInt` byte count.

<a name="string"></a>
#### String Objects

A string object is really nothing more than a [buffer object](#buffer) containing a string encoded in UTF-8.

Binary Encoding:

| Code Byte  | Data            | Python Value                | Class  |
| :--------- | :-------------- | --------------------------: | :----- |
| `01010000` |                 |                          "" | StrObj |
| `01010001` | U `bbbbbbbb`\*U |  bytes(*b…*).decode('utf8') | StrObj |

Here, U represents a `UInt` data-encoded byte count.

<a name="list"></a>
#### List Objects

All list objects begin with an element count, encoded as `UInt` data (without the code byte). Next, in a base `ListObj`, each element is encoded in full (code byte + any object data).

An `SList`, or simple list, is a list in which all elements share the same data type. In this case, a single code byte identifying said type is followed by the data for each element without its code byte.

`SList` handles boolean elements in a special way. It packs 8 to a byte to save space. (See
[Boolean Objects](#bool).)

The auto-optimization logic for lists checks all the elements in your list to see if they share a common type and substitutes an `SList` for a `ListObj` if that is the case. It is smart enough to select the more general data type when necessary. For example, if all elements are unsigned integers, an `SList` of `UInt` will be selected, but if even one of the elements is negative, the element type will be the more general `IntObj` instead.

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
* Each b is a single boolean value for the special case of bools which get packed 8 to a byte with big-endian bit ordering. If U is not a multiple of 8 in this case, the last byte will be padded out with zeros in its least-significant bits.

<a name="dict"></a>
#### Dictionary Objects

The data format for dictionaries looks a lot like two lists stuck together: one for the keys and another for the associated values. Since both lists always share the same length, however, the length gets omitted when the values list is encoded.

Dictionaries have two subtypes: SKDict and SDict. An SKDict is a simple key dictionary in which all keys share the same data type but the values may differ in type. An SDict, or simple dictionary, adds the further restriction that the values must also share the same data type, though this need not be the same type that the keys share.

Binary Encoding:

| Code Byte   | Data                                       | Python Value   | Class   |
| :---------- | :----------------------------------------- | -------------: | :------ |
| `10010000`  |                                            |             {} | DictObj |
| `10010001`† | U K\*U V\*U                                |  {*K*:*V* *…*} | DictObj |
| `10010010`  | U `cccccccc` k\*U V\*U                     |  {*k*:*V* *…*} | SKDict  |
| `10010011`  | U `cccccccc` k\*U `dddddddd` v\*U          |  {*k*:*v* *…*} | SDict   |
| `10010011`  | U `cccccccc` k\*U `00010001` bbbbbbbb\*U/8 |  {*k*:*v* *…*} | SDict   |

Here:
* U is the list element count encoded as a `UInt`'s data without the code byte.
* K represents a full encoding (including the code byte) of each dictionary key.
* V represents a full encoding (including the code byte) of each dictionary
value.
* The c bits represent the code byte shared by all dictionary keys.
* The d bits represent the code byte shared by all dictionary values.
* Lower case k is like K except the code bytes for each key are omitted.
* Lower case v is like V except the code bytes for each value are omitted.
* The b bits indicate boolean values packed 8 to a byte in the same way as they are in an `SList`. (Technically, boolean keys could also get packed this way, but it would be rather silly to use a type that can only be one of two values as your dictionary key.)

*Implementation Note: Though having a single list of key-value pairs may have improved binary legibility, but it would also have eliminated the ability to pack booleans.*

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

As you can see, `BinONObj` is the root class of all the classes that handle specific data types. Each base data type is handled by a class ending in "Obj" in a module of the same name (except that the module name is all in lower case). Any optimized variants of the such classes inherit from the base type's class and can be found in the same module. These specialized classes do not end in "Obj".

<a name="py_high_level"></a>
### High-Level Interface

The easiest way to encode/decode your data in BinON format is to call the `BinONObj.Encode()` and `BinONObj.Decode()` class methods (from the `binon.binonobj` module).

<a name="py_encode"></a>
#### BinONObj.Encode()

`BinONObj.Encode()` expects 2 or 3 arguments:
1. `value` (object): the value to encode
2. `outF` (file object): a writable binary data stream
3. `optimize` (bool, optional): optimize for shorter encoding length where applicable

What you pass in as `value` may be a built-in Python data type.

	BinONObj.Encode(100, outF)
	BinONObj.Encode([1,2,3], outF)

It may also be a BinON class-wrapped value.

	BinONObj.Encode(UInt(100), outF)

More on that under [Low-Level Interface](#py_low_level).

The main thing to remember about `outF` is that it needs to be writable and binary.

	with open("/path/to/foo.bin", "wb") as outF: ...

Alternatively, you could write binary data to the standard output buffer:

	outF = sys.stdout.buffer

A third option would be to write into a `BytesIO` buffer and later call its `getvalue()` method to retrieve the final product.

	outF = io.BytesIO()
	...
	binon = outF.getvalue()

Setting `optimize=True` (it defaults to `False`) tells `Encode()` to look more closely at `value` to determine whether it can apply more optimized subtype classes to encoding to it.

For example, calling `BinONObj.Encode(100, outF)` would ultimately invoke `IntObj(100).encode(outF)`. But `BinONObj.Encode(100, outF, optimize=True)` would invoke `UInt(100).encode(outF)` instead. (For the particular value of 100, this will save 1 byte over the signed integer encoding.)

`optimize` should give you the tightest encoding in most situations, but it comes at a cost in that it has to run over your data once before deciding what to do. This can be particularly expensive with container types. For example, in encoding a list of integers, it would need to check that all elements are indeed integers before substituting an `SList` for a `ListObj`.

Alternatively, you can apply the optimizations manually by wrapping your data in subtype classes yourself as described under the [Low-Level Interface](#py_low_level).

<a name="py_decode"></a>
#### BinONObj.Decode()

`BinONObj.Decode()` expects 1 or 2 arguments:

1. `inF` (file object): a readable binary data stream
2. `asObj` (bool, optional): keep object wrapper on returned value(s)?

Let's try encoding the number 100 and decoding it again.

	>>> outF = io.BytesIO()
	>>> BinONObj.Encode(100, outF, optimize=True)
	>>> binon = outF.getvalue()
	>>> for byte in binon: print(f"0x{byte:02x}")
	...
	0x22
	0x64
	>>> inF = io.BytesIO(binon)
	>>> BinONObj.Decode(inF)
	100
	>>> inF = io.BytesIO(binon)
	>>> uint = BinONObj.Decode(inF, asObj=True)
	>>> uint
	UInt(100)
	>>> uint.value
	100

With optimized encoding, the number only took 2 bytes to encode. `0x22` is a code byte indicating that an integer follows (0x20) and that it is unsigned (0x02). The actual value is in the second `0x64` (== 100) byte.

Then we make these bytes into an input buffer and decode it back to 100. With `asObj=True`, the `UInt` wrapper is left on the decoded value. This option may be useful when you are decoding a larger data structure and want to modify just one part of it before re-encoding it. Having the object wrappers already in place should make it quicker to encode.

<a name="py_low_level"></a>
### Low-Level Interface

The low-level interface involves dealing more directly with the classes that handle specific data types. In particular, you may want to apply optimized data types like `UInt` or `Float32` before encoding them. You can do so at either the scalar level (e.g. `UInt(100)`) or the container level (e.g. `SList([1,2,3], UInt)`).

At this point, you could directly call the `encode()` method directly on the object you just created, or fall back on the higher level `Encode()` class method. The latter may be a good choice if you are only applying optimized classes to parts of your data structure. For example, say you are encoding a data structure containing some specialized elements but not others.

	BinONObj.Encode(["foo", -1, Float32(2.5)], myFile)

This would give you the same output as:

	ListObj([StrObj("foo"), IntObj(-1), Float32(2.5)]).encode(myFile)

`BinONObj.Encode()` supplies base data type classes (those ending in "Obj") for basic Python types automatically, so you need only worry about places where you want a specialized class (one not ending in "Obj") to encode a particular value. Here, if we had simply written `2.5` instead of `Float32(2.5)`, `BinONObj.Encode()` would have supplied `FloatObj(2.5)` instead, which encodes as double-precision (64 bits). (Note that in this particular case, setting `optimize=True` in the `BinONObj.Encode()` call would give you a `Float32` even without the explicit casting, but for floating-point in particular, it is not a great idea to rely on this. See notes on the [floatobj](#float) module.)

<a name="py_encode_data"></a>
#### encodeData() and DecodeData()

The full encoding of a BinON object consists of a code byte indicating the data type followed by a number of bytes of object data. If the data type is obvious from the context of what you are doing, you may omit it by calling the `encodeData()` method on a BinON wrapper instance of the type in question.

For example, `IntObj(42).encode(outF)` should output the hexadecimal: 21 2a. `IntObj(42).encodeData(outF)` would only output: 2a.

`encodeData()` leaves object type identification in your hands, so you must later call the correct counterpart `DecodeData()` method; in this case, `IntObj.DecodeData(inF)`. (Note that BinON follows a convention that class methods begin with a capital letter while instance methods begin in lower case.)

Note that with the exception of `NullObj`, `encodeData()` will always write at least one byte of data. You may recall that with `Encode()`/`encode()` has a special optimization in which the default value for a data type (e.g. 0 for an `IntObj`) need not encode any data since the code byte itself can indicate the value. But `encodeData()` cannot make that optimization since there is no code byte. It will write a `0x00` byte in the case of an integer zero.

<a name="cpp"></a>
## C++ Interface

The C++ interface for BinON is still something of a work in progress,
though it is sufficiently complete/stable that I have started using it in production code at this point.

<a name="cpp_build"></a>
### Building Library

At this point, there is nothing more than a simple gnu makefile to build a static library called libbinon.a. The top few lines give you options to set your compiler of choice and perhaps target c++20 over the default c++17 if your compiler supports it. (Most of the 20-level support in the code base involves using concepts where available.)

You can then `make debug`, `make release`, or `make clean` (to remove all binaries). `make` on its own will build both the debug and release libraries

To use the library, you can simply include the "binon/binon.hpp" header, which in turn includes all of the others. (This header is also touched whenever the binon project is modified, so you can make it a dependency in your own project to make sure things get recompiled when binon gets updated.)

<a name="cpp_structs"></a>
### Data Structures

To begin with, the entire binon interface for C++ is wrapped within the
`namespace binon`.

As with the Python implementation, the C++ defines structures representing the fundamental BinON data types.

* BoolObj
* [BufferObj](#cpp_bufferobj)
* [DictObj](#cpp_dictobj)
* [SKDict](#cpp_dictobj)
* [SDict](#cpp_dictobj)
* FloatObj
* Float32Obj
* [ListObj](#cpp_listobj)
* [SList](#cpp_listobj)
* [IntObj](#cpp_intobj)
* [UIntObj](#cpp_intobj)
* [StrObj](#cpp_strobj)

In comparing these to their Python counterparts, you may notice the latter goes with `UInt` rather than `UIntObj` and `Float32`. This was a design decision bases on the idea that `UInt` and `Float32` may be pretty common names in a lot of C++ libraries and I wanted to prevent naming collisions (even though they are within the `binon` namespace). If you want use `UInt` and `Float32` however, they are defined within `namespace binon::types`.

<a name="cpp_binonobj"></a>
#### BinONObj and Object Helpers

While the Python implementation has a base `BinONObj` class from which all of the specific classes inherit, the C++ implementation builds a `std::variant` around all the above types to produce a `BinONObj`.

Some of the usual suspects from the Python side can be found here, such as the `encode()`, `encodeData()`, and `decodeData()` methods, as well as the `Decode()` class method.

There is no `Encode()` class method, however. You need to produce a `BinONObj` first and then call its `encode()` method.

The easiest way to create a `BinONObj` is to use one of the helper functions defined in "objhelpers.hpp". Though each specific class has an internal data type defined by `TValue` (e.g. `BoolObj::TValue` which is `bool`), the helper functions defined here let you use more traditional C++ types to create or read the value out of objects.

For example, if you wrote:

	auto obj = MakeObj(10U);

`obj` would be the `UIntObj` variant of a `BinONObj` containing the value 10. You could read this value back with:

	auto i = GetObjVal<unsigned int>(obj);

<a name="cpp_bufferobj"></a>
#### BufferObj and BufferVal

The `TValue` type for a `BufferObj` is `BufferVal`. In the current
implementation, this is a HyStr just as in the StrObj case. (In earlier
implementations, it used to be a string of std::byte, but this has been
deprecated on account of std::char_traits not )

`BufferVal`s also print to `std::ostream`s as hexadecimal codes rather
than regular characters.

<a name="cpp_dictobj"></a>
#### DictObj, SKDict, SDict, and Helper Functions

As with [list containers](#cpp_listobj), dictionaries have helper functions that make life easier.

	SDict dict(kStrObjCode, kIntObjCode);
	SetCtnrVal("hello", 1);
	SetCntrVal("world", 2);

<a name="cpp_intobj"></a>
#### IntObj, UIntObj, IntVal, and UIntVal

The `TValue` types for `IntObj` and `UIntObj` are `IntVal` and `UIntVal`, respectively. The latter are also BinON data structures as opposed to a built-in C++ integer type.

The reason for this is that BinON allows for big integers as can appear in Python at times. `IntVal`, then, can contain either a `std::int64_t` or a `std::string` of bytes for cases in which the internal value would not fit in 64 bits.

Look at the documentation in "intobj.hpp" for info on how this works. Of particular note, while both the `scalar()` and `asScalar()` methods of `IntVal` and `UIntVal` return a 64-bit integer, `scalar()` may throw an exception if it must be truncated down from a big integer while `asScalar()` will simply return the truncated int. (Note: helper functions like `GetObjVal()` will use `asScalar()` internally. The idea here is if you wrote `GetObjVal<std::int16_t>(obj)`, it's assumed to behave like a `static_cast<std::int16_t>(val)`, so you are effectively authorizing the truncation.)

<a name="cpp_listobj"></a>
#### ListObj, SList, and Helper Functions

While the `TValue` type for `ListObj` and `SList` are `std::vector<BinONObj>`, you may find it easier to manage individual elements using the helper functions defined in "listhelpers.hpp". For example, you could write:

	SList list(kStrObjCode);
	AppendVal(list, "hello");
	AppendVal(list, "world");

to create a list of strings containing "hello" and "world" elements.

<a name="cpp_strobj"></a>
#### StrObj and HyStr

The `TValue` type for a `StrObj` is a `HyStr`. This is a stand-alone BinON data type standing for "hybrid string". It is essentially a cross between a `std::string` and a `std::string_view`. Read the "hystr.hpp" documentation for details on how it works, but understand the reason why it exists. When you decode a `StrObj`, it will always contain the `std::string` variant of a `HyStr`. But when you are creating one yourself for encoding, it may only need a `std::string_view` for the short time a `std::BinONObj` may exist.
