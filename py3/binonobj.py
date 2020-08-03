"""
This module is the main entry point to the BinON implementation. You can call
class methods BinONObj.Encode() to serialize a data structure and
BinONObj.Decode() to restore it. If you know you are dealing with a specific
data type, the other ...obj modules containing subclasses of BinONObj may offer
a shortcut to encoding/decoding them.
"""

from codebyte import CodeByte

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
	_gTypeBaseCls = {}
	
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
		if isinstance(value, cls):
			return value
		try:
			objCls = cls._gTypeBaseCls[type(value)]
		except KeyError:
			raise cls.TypeErr("BinON cannot encode object of type:".format(
				type(value).__name__
			))
		return objCls._AsObj(value, specialize)
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
		cb = CodeByte.Read(inF)
		try:
			objCls = cls._gCodeObjCls[cb.value]
		except KeyError:
			raise cls.ParseErr(
				"unknown BinON base type {} with subtype {}".format(
					cb.baseType, cb.subtype
				)
			)
		obj = objCls._Decode(cb, inF)
		return obj if asObj else obj.asValue()
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
	def _AsObj(cls, value, specialize):
		return cls(value)
	@classmethod
	def _Decode(cls, codeByte, inF):
		return cls.DecodeData(inF) if cls.subtype else cls()
	
	__slots__ = ["value"]
	
	def __init__(self, value=None):
		self.value = value
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
		st = CodeByte.kBaseSubtype if self.value else CodeByte.kDefaultSubtype
		CodeByte(baseType=self.kBaseType, subtype=st).write(outF)
		if self.value:
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
