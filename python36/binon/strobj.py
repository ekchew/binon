from .binonobj import BinONObj
from .bufferobj import BufferObj

class StrObj(BinONObj):
	"""
	String objects are essentially buffer objects where the string are encoded
	into binary buffers using the UTF-8 format.
	"""
	kBaseType = 5
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		v = BufferObj.DecodeData(inF).decode("utf8")
		return cls(v) if asObj else v
	
	def encodeData(self, outF):
		BufferObj(self.value.encode("utf8")).encodeData(outF)

BinONObj._InitSubcls(StrObj, [], [str])
