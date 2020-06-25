from .codec import Codec, CodeByte
from .ioutil import MustRead

class BoolCodec(Codec):
	kCodecID = Codec.kBoolID
	
	@classmethod
	def Init(cls):
		cls._gTypeCodec[bool] = cls
		cls._gIDCodec[cls.kCodecID] = cls
	@classmethod
	def _EncodeObj(cls, value, outF):
		CodeByte(cls.kCodecID, 0x01 if value else 0x00).write(outF)
	@classmethod
	def _EncodeDataList(cls, lst, outF):
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
	def _DecodeObj(cls, codeByte, inF):
		return codeByte.data != 0x0
	@classmethod
	def _DecodeDataList(cls, inF, size):
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

BoolCodec.Init()
