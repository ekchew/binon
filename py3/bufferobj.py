from .binonobj import BinONObj
from .intobj import UInt
from .ioutil import MustRead

class BufferObj(BinONObj):
	"""
	A buffer object encodes a binary data buffer using an unsigned integer
	(intobj.UInt) specifying its length (in bytes), followed by the actual data.
	"""
	kBaseType = 4
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		v = MustRead(inF, UInt.DecodeData(inF))
		return cls(v) if asObj else v
	
	def encodeData(self, outF):
		UInt(len(self.value)).encodeData(outF)
		outF.write(self.value)

class StrObj(BufferObj):
	"""
	This specialization of BufferObj encodes strings as UTF-8 binary.
	"""
	kSubtype = 2
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		v = BufferObj.DecodeData(inF).decode("utf8")
		return cls(v) if asObj else v
	
	def encodeData(self, outF):
		super().encodeData(self.value.encode("utf8"))

def _Init():
	cb = CodeByte(baseType=BufferObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = BufferObj
	cb.subtype = StrObj.kSubtype
	BinONObj._gCodeObjCls[cb] = StrObj
	for typ in (bytes, bytearray, BufferObj):
		BinONObj._gTypeBaseCls[typ] = BufferObj
	for typ in (str, StrObj):
		BinONObj._gTypeBaseCls[typ] = StrObj

_Init()
