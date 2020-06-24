from .bytescodec import BytesCodec
from .typecodec import TypeCodec

class StrCodec(TypeCodec):
	kTypeID = 0x11
	kTypes = [str]
	
	@staticmethod
	def Encode(value, outFile):
		BytesCodec.Encode(value.encode("utf8"))
	@staticmethod
	def Decode(inFile):
		return BytesCodec.Decode(inFile).decode("utf8")
