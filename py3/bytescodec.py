from .intcodec import IntCodec
from .ioutil import MustRead
from .typecodec import TypeCodec

class BytesCodec(TypeCodec):
	kTypeID = 0x10
	kTypes = [bytearray, bytes]
	
	@staticmethod
	def Encode(value, outFile):
		IntCodec.Encode(len(value), outFile)
		outFile.write(value)
	@staticmethod
	def Decode(inFile):
		n = IntCodec.Decode(inFile)
		return MustRead(inFile, n)
