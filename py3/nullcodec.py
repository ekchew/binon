from .typecodec import TypeCodec

class NullCodec(TypeCodec):
	kValues = [None]
	
	@staticmethod
	def EncodeMultiple(self, values, outFile):
		pass
