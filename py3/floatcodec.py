from .codec import Codec, CodeByte
from .ioutil import MustRead
from struct import Struct

class FloatCls:
	_kInCodeData = {
		0.0: 0x0, 1.0: 0x1, 2.0: 0x2, 0.5: 0x3,
		-0.5: 0x5, -2.0: 0x6, -1.0: 0x7
	}
	_kInCodeValue = {v: k for k, v in _kInCodeData.items()}
	_kStruct = Struct("@f")
	_kStructs = [Struct(f">{c}") for c in "efd"]
	
	@staticmethod
	def _IStructBytes(i):
		return 2 << i
	@classmethod
	def _DecodeObj(cls, inF, codeByte, iStruct):
		if codeByte.data < 8:
			try:
				value = self._kInCodeValue[codeByte.data]
			except KeyError:
				codeByte.raiseParseErr()
		else:
			data = MustRead(inF, cls._IStructBytes(iStruct))
			value = cls._kStructs[iStruct].unpack(data)
		return value
	
	@classmethod
	def FromFloat(cls, value):
		float32 = value == cls._kStruct.unpack(cls._kStruct.pack(value))[0]
		return Float32(value) if float32 else Float64(value)
	
	def __init__(self, value):
		self.value = value
	def _encodeObj(self, outF, codecID, iStruct):
		try:
			cbData = self._kInCodeData[self.value]
		except KeyError:
			CodeByte(codecID, 0x8).write(outF)
			outF.write(self._kStructs[iStruct].pack(self.value))
		else:
			CodeByte(codecID, cbData).write(outF)
class Float16(FloatCls):
	@classmethod
	def DecodeObj(cls, inF, codeByte):
		return cls._DecodeObj(inF, codeByte, iStruct=0)
	@classmethod
	def DecodeData(cls, inF):
		return cls._kStructs[0].unpack(MustRead(inF, 2))
	
	def __init__(self, value):
		super().__init__(value)
	def encodeObj(self, outF):
		self._encode(outF, Codec.kFloat16ID, iStruct=0)
	def encodeData(self, outF):
		outF.write(self._kStructs[0].pack(self.value))	
class Float32(FloatCls):
	@classmethod
	def DecodeObj(cls, inF, codeByte):
		return cls._DecodeObj(inF, codeByte, iStruct=1)
	@classmethod
	def DecodeData(cls, inF):
		return cls._kStructs[1].unpack(MustRead(inF, 4))
	
	def __init__(self, value):
		super().__init__(value)
	def encodeObj(self, outF):
		self._encode(outF, Codec.kFloat32ID, iStruct=1)
	def encodeData(self, outF):
		outF.write(self._kStructs[1].pack(self.value))
class Float64(FloatCls):
	@classmethod
	def DecodeObj(cls, inF, codeByte):
		return cls._DecodeObj(inF, codeByte, iStruct=2)
	@classmethod
	def DecodeData(cls, inF):
		return cls._kStructs[2].unpack(MustRead(inF, 8))
	
	def __init__(self, value):
		super().__init__(value)
	def encodeObj(self, outF):
		self._encode(outF, Codec.kFloat64ID, iStruct=2)
	def encodeData(self, outF):
		outF.write(self._kStructs[2].pack(self.value))

class Float16Codec(Codec):
	_kCodecID = Codec.kFloat16ID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[Float16] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		value.encodeObj(outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		cls._EncodeObjList(lst, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		value.encodeData(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return Float16.DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		return cls._DecodeObjList(inF, length, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		return Float16.DecodeData(inF)
class Float32Codec(Codec):
	_kCodecID = Codec.kFloat16ID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[Float32] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		value.encodeObj(outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		cls._EncodeObjList(lst, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		value.encodeData(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return Float32.DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		return cls._DecodeObjList(inF, length, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		return Float32.DecodeData(inF)
class Float64Codec(Codec):
	_kCodecID = Codec.kFloat16ID
	
	@classmethod
	def _Init(cls):
		for typ in (float, Float64):
			cls._gTypeCodec[typ] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		except AttributeError:
			FloatCls.FromFloat(value).encodeObj(outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		codec = cls
		if lookedUp and lst:
			codec = Float16Codec
			for value in lst:
				if isinstance(value, Float16):
					continue
				if type(value) is float:
					value = FloatCls.FromFloat(value)
				if isinstance(value, Float32):
					codec = Float32
				else:
					codec = cls
					break
		codec._EncodeObjList(lst, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		value.encodeData(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return Float64.DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		return cls._DecodeObjList(inF, length, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		return Float64.DecodeData(inF)

for cls in (Float16Codec, Float32Codec, Float64Codec):
	cls._Init()
