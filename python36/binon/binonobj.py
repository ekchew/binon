"""
This module is the main entry point to the BinON implementation. You can call
class methods BinONObj.Encode() to serialize a data structure and
BinONObj.Decode() to restore it. If you know you are dealing with a specific
data type, the other ...obj modules containing subclasses of BinONObj may offer
a shortcut to encoding/decoding them.
"""

from .codebyte import CodeByte

class BinONObj:
	"""
	BinONObj is the abstract base class of all the other ...Obj classes. It
	includes some class methods for creating instances of the subclasses and
	working with them to encode your data.
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
	
	#	Maps data types onto their object base classes. The data types include
	#	object classes as well as built-in types such as int or list.
	_gTypeCls = {}
	
	#	Class methods to call directly on BinONObj.
	@classmethod
	def AsObj(cls, value, specialize=False):
		"""
		A class method that wraps a value in an object class instance. If the
		value is already an object wrapper (as opposed to a built-in Python data
		type), it will simply be returned.
		
		Encode() calls AsObj() automatically, but if you are in a situation of
		having to re-encode the same values frequently, it can save time to have
		them pre-made into objects (particularly if you set specialize=True).
		
		Args:
			value (object): a built-in data type (or BinONObj subclass instance)
			specialize (bool, optional): specialize object class where possible
				Given a value of built-in data type, AsObj() will return an
				instance of the base class corresponding to that type (e.g.
				intobj.IntObj for an int) by default (specialize=False). If
				specialize is set True, it will examine the value further to see
				if it can be specialized (to say intobj.UInt).

				This may give you a tighter encoding, but at the expense of a
				little more time to prepare the data. If you know which
				specialized classes to use already, it should be faster to
				instantiate the specific class yourself and encode it from
				there.
		
		Returns:
			BinONObj: actually, a subclass of BinONObj
		
		Raises:
			BinONObj.TypeErr: if type of value cannot be handled
		"""
		
		typ = type(value)
		try:
			objCls = cls._gTypeCls[typ]
		except KeyError:
			raise cls.TypeErr(
				f"BinON cannot encode object of type: {typ.__name__}"
			)
		return objCls._AsObj(value, typ is objCls, specialize)
	@classmethod
	def Encode(cls, value, outF, specialize=False):
		"""
		A high-level class method that serializes a value and writes it to a
		binary data stream in BinON format.
		
		Args:
			value (object): any type supported by BinON (see AsObj())
			outF (file object): a writable binary data stream
			specialize (bool, optional): see AsObj()
		"""
		cls.AsObj(value, specialize).encode(outF)
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
	def EncodeSListElems(cls, elems, outF):
		"""
		In an SList (see listobj.py), all elements share the same data type. The
		way we typically encode such elements is to wrap them in the BinON class
		corresponding to that type (if they aren't wrapped already) and call the
		encodeData() method on each object. But this class method (and its
		decoding counterpart) may be overridden where appropriate. For example,
		BoolObj overrides it to pack 8 bools per byte rather than 1 per byte as
		its scalar encodeData() method would normally do. EncodeSListElems()
		should never be called on the base BinONObj class, but rather on one of
		its subclasses handling the appropriate type.
		
		Args:
			elems (iterable of type):
				The type may be any type supported by BinON but all elements
				should be of that type (or at least castable to that type).
			outF (file object): a writable binary data stream
		"""
		for elem in elems:
			try:
				elem.encodeData(outF)
			except AttributeError:
				cls(elem).encodeData(outF)
	@classmethod
	def DecodeSListElems(cls, inF, count, asObj=False):
		"""
		This is the counterpart to EncodeSListElems(). It should never be called
		on the base BinONObj class, but rather on one of its subclasses handling
		the appropriate type. By default, it decodes the specified number of
		element from the input stream.
		
		Args:
			inF (file object): a readable binary data stream
			count (int): the number of elements to read from inF and decode
			asObj (bool, optional): leave return elements in object form?
				E.g. list of IntObj rather than int. See Decode().
		
		Returns:
			list of type:
				The type depends on which which subclass DecodeSListElems() was
				called on (the implied cls arg).
		"""
		return [cls.DecodeData(inF, asObj) for i in range(count)]
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
	def _AsObj(cls, value, isClsObj, specialize):
		return value if isClsObj else cls(value)
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
	def __bool__(self):
		return bool(self.value)
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
