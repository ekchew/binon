from .ioutil import MustRead
from .typecodec import TypeCodec

class BoolCodec(TypeCodec):
	kTypeID = 0x01
	kValues = [False]
	
	@staticmethod
	def Decode(inFile):
		return False
	@staticmethod
	def EncodeMultiple(values, outFile):
		nBits = 0
		byte = 0x00
		for value in values:
			byte <<= 1
			if value:
				byte |= 0x01
			nBits += 1
			if nBits == 8:
				outFile.write(bytes((byte,)))
				byte = 0x00
				nBits = 0
		if nBits:
			outFile.write(bytes((byte << 8 - nBits,)))
	@staticmethod
	def DecodeMultiple(inFile, count):
		data = MustRead(count + 7 >> 3)
		byteIter = iter(data)
		byte = next(byteIter)
		nBits = 8
		values = []
		for i in range(count):
			values.append(True if byte & 0x80 else False)
			nBits -= 1
			if nBits:
				byte <<= 1
			else:
				byte = next(byteIter)
				nBits = 8
		return values
class TrueCodec(BoolCodec):
	kTypeID = 0x02
	kValues = [True]
	
	@staticmethod
	def Decode(inFile):
		return True
