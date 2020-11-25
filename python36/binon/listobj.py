from .binonobj import BinONObj
from .boolobj import BoolObj
from .codebyte import CodeByte
from .intobj import UInt
from .ioutil import MustRead

class ListObj(BinONObj):
	"""
	A base list object encodes the length N of the list as an unsigned integer,
	followed by N elements of fully-formed BinON objects (with a code byte for
	every element).
	"""
	kBaseType = 8
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		count = UInt.DecodeData(inF)
		return cls.DecodeElems(inF, count, asObj)
	@classmethod
	def DecodeElems(cls, inF, count, asObj=False):
		"""
		Recovers list elements encoded by encodeElems().
		
		Args:
			inF (file object): a readable binary data stream
			count (int): number of elements to read
			asObj (bool, optional): leave elements in object form?
				See binonobj.BinONObj.Decode() for more on what this means.
		
		Returns:
			list of object: list of read back elements
		"""
		v = [cls.Decode(inF, asObj) for i in range(count)]
		return cls(v) if asObj else v
	
	@classmethod
	def _OptimalObj(cls, value, inList):
		def subtype(obj):
			return getattr(type(obj), "kSubtype", CodeByte.kBaseSubtype)
		if value:
			value = [cls.OptimalObj(v, inList=True) for v in value]
			pElem = iter(value)
			obj0 = next(pElem)
			stp0 = subtype(obj0)
			try:
				obj1 = next(pElem)
				while obj1.kBaseType == obj0.kBaseType:
					stp1 = subtype(obj1)
					if stp0 > stp1:
						obj0 = obj1
						stp0 = stp1
					obj1 = next(pElem)
			except StopIteration:
				return SList(value, elemCls=type(obj0))
		return cls(value)
	
	def __init__(self, value=()):
		super().__init__(value or [])
	def encodeData(self, outF):
		UInt(len(self.value)).encodeData(outF)
		self.encodeElems(outF)
	def encodeElems(self, outF):
		"""
		Like encodeData() except that it does not write the length of the list
		before the element data. You would need to call DecodeElems() with the
		correct number of elements to recover these.
		
		Args:
			outF (file object): a writable binary data stream
		"""
		for elem in self.value:
			self.Encode(elem, outF)

class SList(ListObj):
	"""
	A simple list is one in which all elements share the same data type. It can
	encode tighter than a general list (ListObj) because it need not include a
	code byte for each element.
	
	The optimize option, where applied to list encoding, scans the list first
	to see if the elements are indeed of the same type, in which case an SList
	is employed (rather than a plain ListObj). Note that if all the elements
	themselves are of the same specialized type, that type will be used.
	Otherwise, the algorithm falls back on the more general type. For example,
	were you encoding a list of floats and all but one were Float32s, it would
	have to go with plain 64-bit FloatObjs for ALL elements to accommodate the
	one exception. (In this case, the simple list may actually encode longer
	than a general one would have, so optimize may not be a great idea. For
	floats, it is generally better to go SList(myArray, Float32) explicitly if
	you mean to write an array of 32-bit floats.)
	"""
	kSubtype = 2
	
	def __init__(self, value, elemCls=None):
		super().__init__(value)
		try:
			self.elemCls = elemCls if elemCls \
				else type(self.BaseObj(value[0]))
		except IndexError:
			raise ValueError("could not determine SList element type")
	def __repr__(self):
		return f"SList({self.value!r}, elemCls={self.elemCls.__name__})"
	
	@classmethod
	def DecodeElems(cls, inF, count, asObj=False):
		cb, objCls = cls._ReadCodeByte(inF)
		if objCls is BoolObj:
			lst = []
			for i in range(count):
				if (i & 0x7) == 0x0:
					byte = MustRead(inF, 1)[0]
				lst.append(True if byte & 0x80 else False)
				byte <<= 1
		else:
			lst = [objCls.DecodeData(inF, asObj) for i in range(count)]
		return lst
	
	def encodeElems(self, outF):
		self.elemCls.GetCodeByte().write(outF)
		if self.elemCls is BoolObj:
			self._encodeBools(outF)
		else:
			for elem in self.value:
				if type(elem) is not self.elemCls:
					if isinstance(elem, BinONObj):
						elem = elem.value
					elem = self.elemCls(elem)
				elem.encodeData(outF)
	def _encodeBools(self, outF):
		byte = 0x00
		for i, elem in enumerate(self.value):
			byte <<= 1
			byte |= 0x01 if elem else 0x00
			if (i & 0x7) == 0x7:
				outF.write(bytes([byte]))
				byte = 0x00
		try:
			i &= 0x7
			if i < 0x7:
				outF.write(bytes([byte << 0x7 - i]))
		except NameError: pass # you can get this if value is empty list

BinONObj._InitSubcls(ListObj, [SList], [list, set, tuple])
