from .codec import Codec, CodeByte
from .intcodec import UInt

class SList:
	def __init__(self, lst, elemCodec):
		self.lst = lst
		self.elemCodec = elemCodec
class GList:
	def __init__(self, lst):
		self.lst = lst

class SListCodec(Codec):
	_kCodecID = Codec._kSListID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[SList] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			UInt(len(value.lst), cls._kCodecID).encodeObj(outF)
			value.elemCodec.EncodeObjList(value.lst, outF)
		except AttributeError:
			UInt(len(value), cls._kCodecID).encodeObj(outF)
			Codec.EncodeObjList(value, outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		cls._EncodeObjList(lst, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		try:
			UInt(len(value.lst)).encodeData(outF)
			value.elemCodec.EncodeObjList(value.lst, outF)
		except AttributeError:
			UInt(len(value)).encodeData(outF)
			Codec.EncodeObjList(value, outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		length = UInt.DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))
		return cls._DecodeList(inF, length)
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		return cls._DecodeObjList(inF, length, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		length = UInt.DecodeData(inF)
		return cls._DecodeList(inF, length)
	
	@classmethod
	def _DecodeList(cls, inF, length):
		codeByte = CodeByte.Read(inF)
		try:
			codec = cls._gIDCodec[codeByte.codecID]
		except KeyError:
			codeByte.raiseParseErr()
		return codec._DecodeObjList(inF, length, codeByte)
class GListCodec(Codec):
	_kCodecID = Codec._kSListID
	
	@classmethod
	def _Init(cls):
		for typ in (list, tuple, set, frozenset, GList):
			cls._gTypeCodec[typ] = cls
		cls._gIDCodec[cls._kCodecID] = cls

for cls in (SListCodec, GListCodec):
	cls._Init()
