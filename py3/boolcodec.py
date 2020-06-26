from .codec import Codec, CodeByte
from .ioutil import MustRead

class BoolCodec(Codec):
	_kCodecID = Codec._kBoolID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[bool] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		CodeByte(cls._kCodecID, 0x01 if value else 0x00).write(outF)
	@classmethod
	def EncodeDataList(cls, lst, outF):
		nBits = 0
		byte = 0x00
		for value in lst:
			byte <<= 1
			if value:
				byte |= 0x01
			nBits += 1
			if nBits == 8:
				outF.write(bytes([byte]))
				byte = 0x00
				nBits = 0
		if nBits:
			outF.write(bytes([byte << 8 - nBits]))
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return cls._CheckCodeByte(codeByte, inF).data != 0x0
	@classmethod
	def DecodeDataList(cls, inF, size):
		data = MustRead(inF, size + 7 >> 3)
		byteIter = iter(data)
		byte = next(byteIter)
		nBits = 8
		values = []
		for i in range(size):
			values.append(True if byte & 0x80 else False)
			nBits -= 1
			if nBits:
				byte <<= 1
			else:
				byte = next(byteIter)
				nBits = 8
		return values

BoolCodec._Init()
