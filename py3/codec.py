from .ioutil import MustRead

class CodeByte:
	@classmethod
	def Read(cls, inF):
		byte = MustRead(inF, 1)[0]
		return cls(byte >> 4 & 0x0F, byte & 0x0F)
	def __init__(self, codecID, data=0x0):
		self.codecID = codecID
		self.data = data
	def write(self, outF):
		outF.write(bytes([self.codecID << 4 & 0xF0 | self.data & 0x0F]))

class Codec:
	kNullID = 0
	kBoolID = 1
	kSIntID = 2
	kUIntID = 3
	kFloatID = 4
	kBytesID = 5
	kStrID = 6
	kSListID = 8
	kGListID = 9
	kSDictID = 10
	kSKDictID = 11
	kGDictID = 12
	
	_gTypeCodec = {}
	_gIDCodec = {}
	
	@classmethod
	def EncodeObj(cls, value, outF):
		value, codec = cls._GetValueCodec(value)
		codec._EncodeObj(value, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		value, valCls = cls._GetValueCodec(value)
		valCls._EncodeData(value, outF)
	@classmethod
	def DecodeObj(cls, inF):
		codeByte = CodeByte.Read(inF)
		return cls._GetCodecFromID(codeByte.codecID)._DecodeObj(codeByte, inF)
	@classmethod
	def DecodeData(cls, codecID):
		return cls._GetCodecFromID(codecID)._DecodeData(inF)
	
	@classmethod
	def _GetCodecFromID(cls, codecID):
		try:
			return cls._gIDCodec[codecID]
		except KeyError:
			raise RuntimeError(f"invalid BinON codec ID: {codecID}")
	@classmethod
	def _GetValueCodec(cls, value):
		try:
			codec = cls._gTypeCodec[type(value)]
		except KeyError:
			raise ValueError(f"BinON cannot encode: {value!r}")
		return codec._ValueCodec(value)
	
	@classmethod
	def _ValueCodec(cls, value):
		return value, cls
	@classmethod
	def _EncodeObj(cls, value, outF):
		CodeByte(cls.kCodecID).write(outF)
	@classmethod
	def _EncodeData(cls, value, outF):
		cls._EncodeObj(value, outF)
	@classmethod
	def _EncodeDataList(cls, lst, outF):
		for value in lst:
			cls._EncodeData(value, outF)
	@classmethod
	def _DecodeObj(cls, codeByte, inF):
		return None
	@classmethod
	def _DecodeData(cls, inF):
		return cls._DecodeObj(CodeByte.Read(inF), inF)
	@classmethod
	def _DecodeDataList(cls, inF, size):
		return [cls._DecodeData(inF) for i in range(size)]
