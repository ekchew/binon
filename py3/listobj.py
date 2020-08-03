from .binonobj import BinONObj
from .codebyte import CodeByte
from .intobj import UInt

class ListObj(BinOnObj):
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
		return [cls.Decode(inF, asObj) for i in range(count)]
	
	@classmethod
	def _AsObj(cls, value, specialize):
		if specialize and len(value) > 1:
			it = iter(value)
			bc = type(cls.AsObj(next(it)))
			sc0 = type(bc._AsObj(value, specialize=True))
			try:
				while True:
					sc = type(cls.AsObj(next(it), specialize=True))
					if not isinstance(sc, bc):
						break
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
		return [objCls.DecodeData(inF, asObj) for i in range(count)]
	
	def encodeElems(self, outF):
		self.elemCls.GetCodeByte().write(outF)
		for elem in self.value:
			self.elemCls._AsObj(elem, specialize=False).encodeData(outF)

def _Init():
	cb = CodeByte(baseType=ListObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = IntObj
	cb.subtype = SList.kSubtype
	BinONObj._gCodeObjCls[cb] = SList
	for typ in (list, set, tuple, ListObj, SList):
		BinONObj._gTypeBaseCls[typ] = ListObj

_Init()
