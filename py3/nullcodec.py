from .codec import Codec, CodeByte

class NullCodec(Codec):
	_kCodecID = Codec._kNullID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[type(None)] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		CodeByte(cls._kCodecID).write(outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		cls.EncodeObj(None, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		pass
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		cls._CheckCodeByte(codeByte)
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		cls.DecodeObj(inF, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		pass

NullCodec._Init()
