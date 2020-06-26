from .codec import Codec

class NullCodec(Codec):
	_kCodecID = Codec._kNullID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[type(None)] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeData(cls, value, outF):
		pass
	@classmethod
	def EncodeDataList(cls, lst, outF):
		pass
	@classmethod
	def DecodeData(cls, inF):
		pass

NullCodec._Init()
