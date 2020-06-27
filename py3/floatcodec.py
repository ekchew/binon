from .codec import Codec, CodeByte
from .ioutil import MustRead
from struct import Struct

class FloatCls:
	"""
	Base class of Float32 and Float64. You should never instantiate this
	directly. You should either allocate a Float32/Float64 or call the
	FromFloat() class method instead.
	
	Attributes:
		value (float): a floating-point value
	"""
	_kInCodeData = {-1.0: 0x7, 0.0: 0x0, 1.0: 0x1}
	_kFStuct = Struct("@f")
	
	@classmethod
	def FromFloat(cls, value):
		"""
		Given a float value, returns either a Float32 or Float64 containing it.
		Basically, it checks to see if it can encode the number as a Float32
		without losing any precision, and uses that if so.
		
		Args:
			value (float): a floating-point value
		
		Returns:
			Float32 or Float64
		"""
		float32 = value == cls._kFStuct.unpack(cls._kFStuct.pack(value))[0]
		return Float32(value) if float32 else Float64(value)
	
	def __init__(self, value):
		self.value = value
	def _encodeInCodeByte(self, outF):
		try:
			data = self._kInCodeData[self.value]
		except KeyError:
			return False
		CodeByte(FloatCodec._kCodecID, data).write(outF)
		return True
class Float32(FloatCls):
	"""
	When FloatCodec encodes a Float32, it write a 32-bit IEEE 754 value
	to the output stream. When it encounters a plain float, it calls
	FromFloat() to see if it can be encoded 32-bit first or else falls
	back on 64. Since FromFloat() is conservative in its approach, it will
	tend to return Float64s much of the time, so if you need only 32-bit
	floats stored, you should wrap them in Float32s yourself or specify
	Float32 as the data type for SLists and such.
	"""
	_kBFStruct = Struct(">Bf")
	def __init__(self, value):
		super().__init__(value)
	def encodeObj(self, outF):
		if not self._encodeInCodeByte(outF):
			codeByte = CodeByte(FloatCodec._kCodecID, 0x9)
			outF.write(self._kBFStruct.pack(codeByte.asByte(), self.value))
class Float64(FloatCls):
	"""
	When FloatCodec encodes a Float32, it write a 64-bit IEEE 754 value
	to the output stream. This tends to be the default when you are writing
	floats to a BinON stream, so you may want to explicitly cast your floats
	to Float32 if that is what you intended.
	"""
	_kBDStruct = Struct(">Bd")
	def __init__(self, value):
		super().__init__(value)
	def encodeObj(self, outF):
		if not self._encodeInCodeByte(outF):
			codeByte = CodeByte(FloatCodec._kCodecID, 0xA)
			outF.write(self._kBDStruct.pack(codeByte.asByte(), self.value))

class FloatCodec(Codec):
	_kCodecID = Codec._kFloatID
	_kInCodeVal = {v: k for k, v in FloatCls._kInCodeData.items()}
	_kF32Val = 0x9
	_kF64Val = 0xa
	_kF32St = Struct(">f")
	_kF64St = Struct(">d")
	
	@classmethod
	def _Init(cls):
		for typ in (float, Float32, Float64):
			cls._gTypeCodec[typ] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		except AttributeError:
			FloatCls.FromFloat(value).encodeObj(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		codeByte = cls._CheckCodeByte(codeByte, inF)
		if (codeByte.data & 0x8) == 0:
			try:
				value = cls._kInCodeVal[codeByte.data]
			except KeyError:
				codeByte.raiseParseErr()
		elif codeByte.data == cls._kF32Val:
			value = cls._kF32St.unpack(MustRead(inF, 4))[0]
		elif codeByte.data == cls._kF64Val:
			value = cls._kF64St.unpack(MustRead(inF, 8))[0]
		else:
			codeByte.raiseParseErr()
		return value

FloatCodec._Init()
