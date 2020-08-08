from .binonobj import BinONObj
from .codebyte import CodeByte
from .ioutil import MustRead
from struct import Struct

class FloatObj(BinONObj):
	kBaseType = 3
	
	_kF32Struct = Struct("f")
	_kF32BEStruct = Struct(">f")
	_kF64BEStruct = Struct(">d")
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		v = cls._kF64BEStruct.unpack(MustRead(inF, 8))
		return cls(v) if asObj else v
	
	@classmethod
	def _AsObj(cls, value, specialize):
		f32OK = specialize \
			and cls._kF32Struct.unpack(cls._kF32Struct.pack(value)) == value
		return Float32(value) if f32OK else FloatObj(value)
	
	def encodeData(self, outF):
		outF.write(self._kF64BEStruct.pack(self.value))

class Float32(FloatObj):
	"""
	The default FloatObj class uses a 64-bit (double-precision) encoding. When
	auto-specialization is in effect, FloatObj will check if its value can be
	represented in 32 bits without a loss of precision, in which case it will
	use a Float32. This check will likely fail for a lot of values, however, so
	if you want 32-bit floats, you are advised to ask for them explicitly (by
	either casting your values to Float32 manually or specifying it as the class
	to use with container classes like SList).
	"""
	kSubtype = 2
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		v = cls._kF32BEStruct.unpack(MustRead(inF, 4))
		return cls(v) if asObj else v
	
	def encodeData(self, outF):
		outF.write(self._kF32BEStruct.pack(self.value))

def _Init():
	cb = CodeByte(baseType=FloatObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = FloatObj
	cb.subtype = Float32.kSubtype
	BinONObj._gCodeObjCls[cb] = Float32
	for typ in (float, FloatObj, Float32):
		BinONObj._gTypeBaseCls[typ] = FloatObj

_Init()