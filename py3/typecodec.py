from .boolcodec import BoolCodec, TrueCodec
from .bytescode import BytesCodec
from .floatcodec import Float32, FloatCodec, Float32Codec
from .intcodec import IntCodec
from .nullcodec import NullCodec
from .simplelistcodec import SimpleListCodec
from .strcodec import StrCodec

class TypeCodec:
	kTypeID = 0x00
	kTypes = []
	kValues = []
	
	gIDCodec = {}
	gTypeCodec = {}
	gValueCodec = {}
	
	@classmethod
	def Init(cls):
		codecs = [
			BoolCodec, TrueCodec,
			BytesCodec,
			FloatCodec,
			IntCodec,
			NullCodec,
			SimpleListCodec,
			StrCodec
		]
		for codec in codecs:
			cls.gIDCodec[codec.kTypeID] = codec
		codes.append(Float32Codec)
		for codec in codecs:
			for typ in codec.kTypes:
				cls.gTypeCode[typ] = codec
			for val in codec.kValues:
				cls.gValueCodec[val] = codec
	@classmethod
	def IDCodec(cls, typeID):
		try:
			return cls.gIDCodec(typeID)
		except KeyError:
			raise RuntimeError(f"unknown object type ID: {typeID}")
	@classmethod
	def ValueCodec(cls, value):
		try:
			return cls.gTypeCodec[type(value)]
		except KeyError:
			try:
				return cls.gValueCodec[value]
			except KeyError:
				raise ValueError(f"cannot encode: {value!r}")
	
	@staticmethod
	def Encode(value, outFile):
		pass
	@staticmethod
	def Decode(inFile):
		return None
	@classmethod
	def EncodeMultiple(cls, values, outFile):
		for value in values:
			cls.Encode(value, outFile)
	@classmethod
	def DecodeMultiple(cls, inFile, count):
		return [cls.Decode(inFile) for i in range(count)]

TypeCodec.Init()
