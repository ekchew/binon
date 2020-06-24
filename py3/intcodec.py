from .ioutil import MustRead
from .typecodec import TypeCodec

import struct

class IntCodec(TypeCodec):
	kTypeID = 0x03
	kTypes = [int]
	
	_kStructs = [struct.Struct(f">{c}") for c in "BHIQ"]
	_kStructS64 = struct.Struct(">q")
	_kStructBS64 = struct.Struct(">Bq")
	_kStructBU64 = struct.Struct(">BQ")
	
	@classmethod
	def Encode(cls, value, outFile):
		mag = value = -1 - value if value < 0 else value
		iStruct = 0
		sigBitsB = 6
		magLimit = 1 << 6
		while mag >= magLimit:
			iStruct += 1
			sigBitsA = sigBitsB
			sigBitsB <<= 1
			sigBitsB += iStruct
			magLimit <<= sigBitsB - sigBitsA
		try:
			structI = cls._kStructs[iStruct]
		except IndexError:
			if mag < 1 << 63:
				outFile.write(cls._kStructBS64.pack(0xfc, value))
			elif value >= 0 and mag < 1 << 64:
				outFile.write(cls._kStructBU64.pack(0xfd, value))
			else:
				code = 0xfe if value < 0 else 0xff
				outfile.write(bytes((code,)))
				while sigBitsA != sigBitsB:
					sigBitsM = sigBitsA + sigBitsB >> 1
					if mag > 1 << sigBitsM:
						sigBitsA = sigBitsM
					else:
						sigBitsB = sigBitsM
				nBits = sigBitsA + 1 if value < 0 else sigBitsA
				value &= (1 << nBits) - 1
				nBytes = nBits + 7 >> 3
				cls.Encode(nBytes - 9, outFile)
				outFile.write(bytes(
					value >> i & 0xff
						for i in range((nBytes << 3) - 8, -8, -8)
				))
		else:
			value &= (magLimit << 1) - 1
			value |= (1 << i) - 1 << sigBitsB + 2
			outFile.write(structI.pack(value))
	@classmethod
	def Decode(cls, inFile):
		data = bytearray(MustRead(inFile, 1))
		code = data[0]
		for iStruct in range(len(cls._kStructs)):
			n = (1 << iStruct) - 1
			if code >> 7 - iStruct == n << 1:
				if n:
					data.extend(MustRead(inFile, n))
				value = cls._kStructs[iStruct].unpack(data)
				limit = 1 << (8 << iStruct) - iStruct - 1
				value &= limit - 1
				break
		else:
			if code & 0x02:
				n = cls.Decode(inFile)
				data = MustRead(inFile, n)
				value = 0
				for byte in data:
					value <<= 8
					value |= byte
				if code & 0x01:
					return value
				limit = 8 << n
			else:
				data = MustRead(inFile, 8)
				st = cls._kStructs[3] if code & 0x01 else cls._kStructS64
				return st.unpack(data)
		if value > limit >> 1:
			value -= limit
		return value
