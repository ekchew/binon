from .binonobj import BinONObj
from .boolobj import BoolObj
from .codebyte import CodeByte
from .intobj import UInt
from .ioutil import MustRead

class ListObj(BinONObj):
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
	def _AsObj(cls, value, specialize):
		if specialize and len(value) > 1:
			it = iter(value)
			bc = type(cls.AsObj(next(it)))
			sc0 = type(bc._AsObj(value, specialize=True))
			try:
				while True:
					obj = cls.AsObj(next(it), specialize=True)
					if not isinstance(obj, bc):
						break
					sc = type(obj)
					if sc is not sc0:
						sc0 = bc
			except StopIteration: pass
			else:
				return SList(value, sc0)
		return cls(value)
	
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
	
	The specialize option, where applied to list encoding, scans the list first
	to see if the elements are indeed of the same type, in which case an SList
	is employed (rather than a plain ListObj). Note that if all the elements
	themselves are of the same specialized type, that type will be used.
	Otherwise, the algorithm falls back on the more general type. For example,
	were you encoding a list of floats and all but one were Float32s, it would
	have to go with plain 64-bit FloatObjs for ALL elements to accommodate the
	one exception. (In this case, the simple list may actually encode longer
	than a general one would have, so specialize may not be a great idea. For
	floats, it is generally better to go SList(myArray, Float32) explicitly if
	you mean to write an array of 32-bit floats.)
	"""
	kSubtype = 2
	
	def __init__(self, value, elemCls=None):
		super().__init__(value)
		try:
			self.elemCls = elemCls if elemCls else type(self.AsObj(value[0]))
		except IndexError:
			raise ValueError("could not determine SList element type")
	
	@classmethod
	def DecodeElems(cls, inF, count, asObj=False):
		objCls = cls._ReadCodeByte(inF)
		if objCls is BoolObj:
			lst = []
			for i in range(count):
				if (i & 0x7) == 0x0:
					byte = MustRead(inF, 1)[0]
				lst.append(True if byte & 0x80 else False)
				byte <<= 1
		else lst = [objCls.DecodeData(inF, asObj) for i in range(count)]
		return lst
	
	def encodeElems(self, outF):
		if self.elemCls is BoolObj:
			self._encodeBools(outF)
		else:
			self.elemCls.GetCodeByte().write(outF)
			for elem in self.value:
				self.elemCls._AsObj(elem, specialize=False).encodeData(outF)
	def _encodeBools(self, outF):
		byte = 0x00
		for i, elem in enumerate(self.value):
			byte <<= 1
			byte |= 0x01 if elem else 0x00
			if (i & 0x7) == 0x7:
				outF.write([byte])
				byte = 0x00
		try:
			i &= 0x7
			if i < 0x7:
				outF.write([byte << 0x7 - i])
		except NameError: pass # you can get this if value is empty list

def _Init():
	cb = CodeByte(baseType=ListObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = IntObj
	cb.subtype = SList.kSubtype
	BinONObj._gCodeObjCls[cb] = SList
	for typ in (list, set, tuple, ListObj, SList):
		BinONObj._gTypeBaseCls[typ] = ListObj

_Init()
