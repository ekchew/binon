from .codec import Codec

class NullCodec(Codec):
	kCodecID = Codec.kNullID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[type(None)] = cls
		cls._gIDCodec[cls.kCodecID] = cls
	@classmethod
	def _EncodeData(cls, value, outF):
		pass
	@classmethod
	def _EncodeDataList(cls, lst, outF):
		pass
	@classmethod
	def _DecodeData(cls, inF):
		pass

NullCodec._Init()
