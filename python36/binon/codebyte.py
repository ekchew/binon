from .ioutil import MustRead

class CodeByte:
	"""
	Typically, the BinON format encodes a code byte followed by the data for a
	given object. The code byte identifies the data type of the object. It
	consists of a base type in the most-significant nibble (4 bits) and a
	subtype in the least-significant nibble.

	The base type has a one-to-one mapping with Python's built-in types such as
	int or list. The subtypes can handle specializations to these types such as
	UInt and SList. They can also handle a default state that requires no extra
	data bytes to encode. The default state is whatever makes a value evaluate
	to False for the sake of conditional expressions. So it would be False for a
	bool, 0 for an int, "" for a str, [] for a list, and so forth.
	
	Class Attributes:
		kDefaultSubtype (0): subtype indicating the default state (see above)
		kBaseSubtype (1): subtype indicating an unspecialized base type
			In other words, the object is an IntObj, FloatObj, etc. rather than
			a UInt, Float32, etc. Specialized subtypes start at 2 and up and
			equates for these are defined in other ...obj modules since they are
			context-dependent on the base type.
	
	Instance Attributes:
		value (int): should be a number in the range [0,255]
	
	Properties:
		baseType (int, read/write): e.g. the float base type is 3
		subtype (int, read/write): e.g. the Float32 subtype is 2
	"""
	kDefaultSubtype = 0
	kBaseSubtype = 1
	
	@classmethod
	def BaseSubtypes(cls):
		"""
		Simply returns the two subtypes associated with the base object class in
		tuple form. (This is used interally by the object classes in registering
		themselves into BinONObj's internal database.)
		
		Returns:
			tuple of (int, int): (kDefaultSubtype, kBaseSubtype)
		"""
		return cls.kDefaultSubtype, cls.kBaseSubtype
	@classmethod
	def Read(cls, inF):
		"""
		Reads a code byte from a file or other data stream.
		
		Args:
			inF (file object): a readable binary data stream
		
		Returns:
			CodeByte: the read-in code byte
		
		Raises:
			ioutil.EndOfFile: on failure to read the code byte
		"""
		return cls(MustRead(inF, 1)[0])
	
	__slots__ = ["value"]
	
	@property
	def baseType(self):
		return self.value >> 4 & 0x0F
	@baseType.setter
	def baseType(self, v):
		self.value = self.value & 0x0F | (v & 0xF) << 4
	@property
	def subtype(self):
		return self.value & 0x0F
	@subtype.setter
	def subtype(self, v):
		self.value = self.value & 0xF0 | v & 0x0F
	
	def __init__(self, value=0x00, baseType=None, subtype=None):
		"""
		Args:
			value (int, optional): integer in the range [0,255]
				Defaults to zero.
			baseType (int or None, optional): integer in the range [0,15]
				If you supply an int in this keyword-only argument, it will
				override the codec ID bits of the value byte. Defaults to None.
			subtype (int or None, optional): integer in the range [0,15]
				If you supply an int in this keyword-only argument, it will
				override the data bits of the value byte. Defaults to None.
		"""
		self.value = value
		if baseType is not None:
			self.baseType = baseType
		if subtype is not None:
			self.subtype = subtype
	def __repr__(self):
		return f"CodeByte(baseType={self.baseType}, subtype={self.subtype})"
	def __hash__(self):
		return hash(self.value)
	def __eq__(self, rhs):
		return self.value == rhs.value
	
	def write(self, outF):
		"""
		Writes the code byte to a file or other data stream.
		
		Args:
			outF (file object): a writable binary data stream
		"""
		outF.write(bytes([self.value]))
