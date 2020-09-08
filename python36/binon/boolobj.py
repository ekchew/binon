from .binonobj import BinONObj
from .codebyte import CodeByte
from .ioutil import MustRead

class BoolObj(BinONObj):
	kBaseType = 1
	
	@classmethod
	def _AsObj(cls, value, specialize):
		if specialize and value:
			return TrueObj(True)
		return cls(value)
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		v = bool(MustRead(inF, 1)[0])
		return cls(v) if asObj else v
	
	def encode(self, outF):
		"""
		Though not strictly necessary, BoolObj's encode() method has been
		overridden to write a more condensed TrueObj() where applicable, even
		without the specialize option set True in BinONObj.Encode().
		"""
		if self.value:
			TrueObj(self.value).encode(outF)
		else:
			super().encode(outF)
	def encodeData(self, outF):
		outF.write(b"\1" if self.value else b"\0")

class TrueObj(BoolObj):
	kSubtype = 2
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		return cls(True) if asObj else True
	
	def __init__(self):
		super().__init__(True)
	
	def encodeData(self, outF): pass

def _Init():
	cb = CodeByte(baseType=BoolObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = BoolObj
	cb.subtype = TrueObj.kSubtype
	BinONObj._gCodeObjCls[cb] = TrueObj
	for typ in (bool, BoolObj, TrueObj):
		BinONObj._gTypeBaseCls[typ] = BoolObj

_Init()
