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

def _Init():
	cb = CodeByte(baseType=BufferObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = BufferObj
	for typ in (bytes, bytearray, BufferObj):
		BinONObj._gTypeBaseCls[typ] = BufferObj

_Init()
