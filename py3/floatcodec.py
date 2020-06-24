from .ioutil import MustRead
from .typecodec import TypeCodec

import struct

class Float32:
	def __init__(self, value):
		self.value = value

class FloatCodec(TypeCodec):
	kTypeID = 0x04
	kTypes = [float]
	
	_kStructBF32 = struct.Struct(">Bf")
	_kStructBF64 = struct.Struct(">Bd")
	_kStructs = {n : struct.Struct(f">{c}") for n, c in zip((4, 8), "fd")}
	
	@classmethod
	def Encode(cls, value, outFile):
		outFile.write(cls._kStructBF64.pack(0x08, value))
	@classmethod
	def Decode(cls, inFile):
		n = MustRead(inFile, 1)
		try:
			st = cls._kStructs[n]
		except KeyError:
			raise RuntimeError(f"float must be 4 or 8 bytes, not {n}")
		return st.unpack(MustRead(inFile, n))
class Float32Codec(FloatCodec):
	kTypes = [Float32]
	
	@classmethod
	def Encode(cls, float32, outFile):
		outFile.write(cls._kStructBF32.pack(0x04, float32.value))
