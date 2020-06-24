from .intcodec import IntCodec
from .ioutil import MustRead
from .nullcodec import NullCodec
from .typecodec import TypeCodec

class SimpleList:
	def __init__(self, lst, codec=None):
		self.lst = lst
		self.codec = codec if codec else (
			TypeCodec.ValueCodec(lst[0]) if lst else NullCodec
		)

class SimpleListCodec(TypeCodec):
	kTypeID = 0x00
	kTypes = [SimpleList]
	
	@classmethod
	def Encode(cls, simpleList, outFile):
		IntCodec.Encode(len(simpleList.lst))
		outFile.write(bytes(simpleLst.typeID,))
		simpleList.codec.EncodeMultiple(simpleList.lst, outFile)
	@classmethod
	def Decode(cls, inFile):
		count = IntCodec.Decode(inFile)
		typeID = MustRead(inFile, 1)[0]
		codec = cls.IDCodec(typeID)
		return codec.DecodeMultiple(inFile, count)
