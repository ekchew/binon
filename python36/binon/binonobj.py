"""
This module is the main entry point to the BinON implementation. You can call
class methods BinONObj.Encode() to serialize a data structure and
BinONObj.Decode() to restore it. If you know you are dealing with a specific
data type, the other ...obj modules containing subclasses of BinONObj may offer
a shortcut to encoding/decoding them.

Attributes:
	gOptimizeLog (file object or None):
		Setting this a writable text stream if you want some diagnostics on
		what the optimization algorithm is doing. The idea here is that if
		you have a fairly static data structure, you can watch how it is
		being optimized and then do it manually to save time if that matters
		to you. The default None disables the logging.
"""

from .codebyte import CodeByte
from contextlib import contextmanager
import traceback, sys

class BinONObj:
	"""
	BinONObj is the abstract base class of all the other ...Obj classes. It
	includes some class methods for creating instances of the subclasses and
	working with them to encode your data.
	
	Attributes:
		value (object): value wrapped by BinONObj subclass.
	"""
	
	class ParseErr(RuntimeError):
		"""
		Raised when something goes wrong in decoding a BinON data stream.
		"""
		pass
	class TypeErr(ValueError):
		"""
		Raised when you try to encode a data type unknown to BinON.
		"""
		pass
	
	#	BinON uses a big-endian byte order
	_kEndian = "big"
	
	#	Maps CodeByte values onto their corresponding object classes.
	_gCodeObjCls = {}
	
	#	Maps data types onto their object basic classes. The data types include
	#	object classes as well as built-in types such as int or list.
	_gTypeCls = {}
	
	#	Class methods to call directly on BinONObj.
	@classmethod
	def BaseObj(cls, value):
		"""
		Before you can encode a given value in BinON, you need to identify which
		codec class can handle values of that type. The codec classes are
		subclasses of BinONObj such as IntObj, StrObj, etc.
		
		BaseObj() looks up the most general codec class that can handle your
		value. This look-up is quick but the class returned is not necessarily
		optimal. For example, BinONObj.BaseObj(100) will return an IntObj,
		but a UInt (a subclass of IntObj) would give you a shorter encoding. If
		you want a UInt instead, you can either call OptimalObj() or simply wrap
		your value in a UInt manually.
		
		BaseObj() is typically called internally by Encode() when the optimize
		option is False.
		
		Args:
			value (object): a value you want to encode
				This can be an instance of either a built-in type or a subclass
				of BinONObj. (In the latter case, value will be returned as-is.)
		
		Returns:
			type: a subclass of BinOnObj
				This is the most general BinOnObj subclass that can encode your
				value. For example, BinOnObj.BaseObj(100) would return
				IntObj.
		
		Raises:
			BinONObj.TypeErr: value is of a type that cannot be encoded
		"""
		try:
			return value if isinstance(value, BinONObj) \
				else cls._gTypeCls[type(value)](value)
		except KeyError:
			raise cls.TypeErr(
				f"BinON cannot encode object of type: {type(value).__name__}"
			)
	@classmethod
	def OptimalObj(cls, value, inList=False):
		"""
		OptimalObj() tries to find the best possible codec class for your
		value by examining it more closely than BaseObj() does. For
		example, it would realize that 100 would be better encoded as a UInt
		than a more general signed IntObj.
		
		This optimization does incur some extra processing overhead, however,
		particularly when dealing with container types (e.g. determining whether
		an SList could be substituted for a ListObj).
		
		If you know what you are dealing with already, you can apply the codec
		manually to save some time. For example, rather than going
		BinONObj.OptimalObj([1, 2, 3]), you could wrap your list manually in
		the appropriate codec class with listobj.SList([1, 2, 3], intobj.UInt).
		
		(Note that for floating-point values, it is often better to explicitly
		wrap your numbers in floatobj.Float32 if you know they are
		single-precision, since the algorithm OptimalObj() uses in this case is
		not great.)
		
		OptimalObj() is typically called internally by Encode() when the
		optimize option is True.
		
		Args:
			value (object): a value you want to encode
				This can be an instance of either a built-in type or a subclass
				of BinONObj.
			inList (bool, optional): value is a list element?
				Generally, this option is used internally and you can leave it
				with False by default. It is pertinent to the case in which your
				value is the default value for the codec in question. For
				example, BinONObj.OptimalObj(0.0) should return a FloatObj.
				Normally, this general codec writes it's value in 64-bit
				double-precision, but for the special case of the default 0.0
				value, it can avoid writing anything at all besides the code
				byte. But if your 0.0 is part of a simple list (listobj.SList)
				of 32-bit floats, there is no need for 0.0 to require 64 bits,
				so BinONObj.OptimalObj(0.0, inList=True) should return a Float32
				instead.
		
		Returns:
			type: a subclass of BinOnObj
				This is the most optimal BinOnObj subclass that can be
				determined algorithmically to encode your value. For example,
				BinOnObj.OptimalObj(100) would return UInt.
		"""
		if cls._OptimizeLog():
			with cls._OptimizeLogIndent() as indent:
				print(
					indent + "Assessing for optimization:", repr(value),
					file=cls._OptimizeLog()
				)
				return value if isinstance(value, BinONObj) \
					else cls.BaseObj(value)._OptimalObj(value, inList)
		return value if isinstance(value, BinONObj) \
			else cls.BaseObj(value)._OptimalObj(value, inList)
	@classmethod
	def Encode(cls, value, outF, optimize=False):
		"""
		A high-level class method that serializes a value and writes it to a
		binary data stream in BinON format.
		
		Args:
			value (object): any type supported by BinON
			outF (file object): a writable binary data stream
			optimize (bool, optional): see OptimalObj()
		"""
		obj = cls.OptimalObj(value) if optimize else cls.BaseObj(value)
		obj.encode(outF)
	@classmethod
	def Decode(cls, inF, asObj=False):
		"""
		Decodes a value encoded by Encode().
		
		Args:
			inF (file object): a readable binary data stream
			asObj (bool, optional): leave return value in object form?
				By default (False), the object wrapper (e.g. IntObj) is removed
				from the return value (giving you an int). Setting asObj True
				leaves the wrapper intact. This could save some time if you need
				to re-encode sometime later?
		
		Returns:
			object: the decoded object
		
		Raises:
			ioutil.EndOfFile: if the object data cannot be read
			BinONObj.ParseErr: if there is something wrong with the object data
		"""
		cb, objCls = cls._ReadCodeByte(inF)
		if cb.subtype == CodeByte.kDefaultSubtype:
			obj = objCls()
			return obj if asObj else obj.value
		return objCls.DecodeData(inF, asObj)
	
	#	Class methods to be called only on subclasses of BinONObj.
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		"""
		Decodes an object's data representation as encoded by encodeData().
		
		WARNING:
			Do not call this class method on the base BinIOObj class! You must
			call it on the appropriate subclass instead.
		
		Args:
			inF (file object): a readable binary data stream
			asObj (bool, optional): leave return value in object form?
				See Decode().
		
		Returns:
			object: the decoded object
		"""
		obj = cls()
		return obj if asObj else obj.asValue()
	@classmethod
	def GetCodeByte(cls):
		"""
		Given a subclass of BinONObj, this method returns a CodeByte with its
		baseType and subtype fields filled out appropriately.
		
		For base type classes, the subtype will be set to CodeByte.kBaseSubtype
		(as opposed to CodeByte.kDefaultSubtype).
		
		Returns:
			CodeByte: the code byte for the BinONObj subclass in question
		"""
		try:
			return CodeByte(
				baseType = cls.kBaseType,
				subtype = cls.kSubtype
			)
		except AttributeError:
			return CodeByte(
				baseType = cls.kBaseType,
				subtype = CodeByte.kBaseSubtype
			)
	
	#	Private class methods (may or may not be overridden).
	@classmethod
	def _OptimalObj(cls, value, inList):
		#	This method is called by the public OptimalObj(), which should have
		#	determined by this point what binon base class is suitable for the
		#	given value. By default, _OptimalObj() simply returns its value
		#	wrapped in base class, but it may be overriden to perform extra
		#	optimization checks and possibly substitute a subclass instead. In
		#	other words, _OptimalObj() does the actual optimizing work.
		#
		#	Args:
		#		cls (type): appropriate base class wrapper for value
		#		value (object): a raw value not wrapped in a binon class
		#		inList (bool): value to be considered part of a list?
		#
		#	Returns:
		#		binon class-wrapped value
		
		return cls(value)
	@classmethod
	def _InitSubcls(cls, baseCls, specClsLst, pyTypes):
		#	Each subclass module calls this method to register its classes into
		#	the two dictionaries the root BinONOBj class maintains to identify
		#	them by either data type (for encoding) or code byte (for
		#	decoding). The two dictionaries are _gTypeCls and _gCodeObjCls,
		#	respectively.
		#	
		#	Note also that the __init__.py file for the binon package
		#	automatically imports all the modules whose names end in "obj"
		#	(aside from binonobj itself) in order to make sure every type gets
		#	registered. So these "...obj" modules can sort of be thought of as
		#	plug-ins that automatically get loaded by the binon package.
		#
		#	Args:
		#		baseCls (type): base class of a plug-in module
		#			This is the class whose name matches that of the module
		#			(other than it being in camel case), such as intobj.IntObj.
		#		specClsLst (iterable of type): specialized classes in module
		#			e.g. [intobj.UInt].
		#		pyTypes (iterable of type): Python types handled by module
		#			e.g. [int]
		
		#	The relevant built-in Python types plus the base class should all
		#	map onto the base class for the purposes of encoding. (In the
		#	former case, a specialized class may get substituted later if the
		#	specialization flag is set True.)
		for typ in pyTypes + [baseCls]:
			cls._gTypeCls[typ] = baseCls
		
		#	On the decoding side, there are two standard subtype codes in the
		#	code byte that should map onto the base type:
		#	CodeByte.kDefaultSubtype and CodeByte.kBaseSubtype.
		#	CodeByte.BaseSubtypes() is a convenience class method to let you
		#	iterate those two.
		for bst in CodeByte.BaseSubtypes():
			
			#	Note that every base class defines a kBaseType (e.g. 2 for
			#	IntObj).
			cb = CodeByte(baseType=baseCls.kBaseType, subtype=bst)
			cls._gCodeObjCls[cb] = baseCls
		
		for typ in specClsLst:
			
			#	If a value has already been wrapped in a specialized class
			#	(e.g. UInt(42)), that is obviously the class we will want to be
			#	using.
			cls._gTypeCls[typ] = typ
			
			#	Every specialized class should define a kSubtype that is >= 2
			#	(since 0 and 1 are reserved for kDefaultSubtype and
			#	kBaseSubtype, respectively). We match this subtype to class in
			#	question.
			cb = CodeByte(baseType=baseCls.kBaseType, subtype=typ.kSubtype)
			cls._gCodeObjCls[cb] = typ
	@classmethod
	def _ReadCodeByte(cls, inF):
		#	Returns code byte and object class looked up it.
		cb = CodeByte.Read(inF)
		try:
			return cb, cls._gCodeObjCls[cb]
		except KeyError:
			typeInfo = f"base type = {cb.baseType}, subtype = {cb.subtype}"
			raise cls.ParseErr(f"unknown BinON data type: {typeInfo}")
	
	__slots__ = ["value"]
	
	def __init__(self, value=None):
		self.value = value
	def __repr__(self):
		return f"{type(self).__name__}({self.value!r})"
	def __bool__(self):
		return bool(self.value)
	def __hash__(self):
		return hash(self.value)
	def __eq__(self, rhs):
		return self.value == rhs.value
	
	def asValue(self):
		"""
		Returns:
			The value contained within the object. For container types, this
			differs from simply accessing the .value attribute in that it will
			remove any object wrappers around child elements as well.
		"""
		return self.value
	def encode(self, outF):
		"""
		A lower-level version of Encode() where we know already the exact data
		format in which to encode the enclosed value.
		
		Args:
			outF (file object): a writable binary data stream
		"""
		cb = self.GetCodeByte()
		if cb.subtype == CodeByte.kBaseSubtype and not self.value:
			cb.subtype = CodeByte.kDefaultSubtype
		cb.write(outF)
		if cb.subtype != CodeByte.kDefaultSubtype:
			self.encodeData(outF)
	def encodeData(self, outF):
		"""
		Every BinON object is represented by a code byte which is typically
		followed by object data. This method foregoes writing the code byte and
		jumps straight to the data.

		Strictly speaking then, calling encodeData() will not generate legit
		BinON, but you may find use cases where you know by context what the
		data type is and don't need a code byte to tell you. You must, however,
		call the correct DecodeData method to get your object back. For example,
		if you called intobj.UInt(42).encodeData(outF), you need to call
		intobj.UInt.DecodeData(inF) to get your 42 back.
		
		Args:
			outF (file object): a writable binary data stream
		"""
		pass
	
	@classmethod
	def _OptimizeLog(cls):
		#	Convenience method to make gOptimizeLog easily accessible to
		#	the BinONObj subclasses.
		return gOptimizeLog
	@classmethod
	@contextmanager
	def _OptimizeLogIndent(cls):
		#	Context manager of indent levels for printing optimization info on
		#	hierarchical data structures.
		global _gIndent
		_gIndent += 1
		try:
			yield cls._IndentStr()
		finally:
			_gIndent -= 1
	@classmethod
	def _IndentStr(cls):
		#	Returns a string of _gIndent tab characters.
		return "\t" * _gIndent

gOptimizeLog = None
_gIndent = -1
