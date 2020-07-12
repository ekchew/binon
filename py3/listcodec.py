from .codec import Codec, CodeByte
from .intcodec import UInt

class ListCls:
	@classmethod
	def FromSeq(cls, seq):
		it = iter(seq)
		try:
			val0 = next(it)
		except StopIteration:
			pass
		else:
			typ0 = type(val0)
			try:
				while type(next(it)) is typ0:
					pass
			except StopIteration:
				return SList(seq, Codec.CodecFromValue(val0))
		return GList(seq)
	
	def __init__(self, lst):
		self.lst = lst
	def _encodeObjLen(self, outF):
		UInt(len(self.lst), self._kCodecID).encodeObj(outF)
class SList(ListCls):
	_kCodecID = Codec._kSListID
	
	def __init__(self, lst, elemCodec):
		super().__init__(lst)
		self.elemCodec = elemCodec
	def encodeObj(self, outF):
		self._encodeObjLen(outF)
		value.elemCodec.EncodeObjList(value.lst, outF)
class GList(ListCls):
	_kCodecID = Codec._kGListID
	
	def __init__(self, lst):
		super().__init__(lst)
	def encodeObj(self, outF):
		self._encodeObjLen(outF)
		for elem in self.lst:
			Codec.EncodeObj(elem, outF)

class SListCodec(Codec):
	_kCodecID = SList._kCodecID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[SList] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		except AttributeError:
			ListCls.FromSeq(value).encodeObj(outF)
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
	_kCodecID = GList._kCodecID
	
	@classmethod
	def _Init(cls):
		for typ in (list, tuple, set, frozenset, GList):
			cls._gTypeCodec[typ] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		except AttributeError:
			ListCls.FromSeq(value).encodeObj(outF)

for cls in (SListCodec, GListCodec):
	cls._Init()
