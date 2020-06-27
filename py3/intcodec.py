from .codec import Codec, CodeByte
from .ioutil import MustRead
from struct import Struct

class IntCls:
	_kFormatChrs = "BHIQ"
	_kIStructs = {Struct(f">{c}") for c in _kFormatChrs}
	_kBIStructs = {Struct(f">B{c}") for c in _kFormatChrs}
	
	@classmethod
	def FromInt(cls, value):
		return SInt(value) if value < 0 else UInt(value)
	
	def __init__(self, value):
		self.value = value
class SInt(IntCls):
	def __init__(self, value):
		super().__init__(value)
class UInt(IntCls):
	def __init__(self, value):
		super().__init__(value)
	def encodeObj(self, outF):
		i = self._objEncoding()
		if i < 0:
			CodeByte(UIntCodec._kCodecID, self.value).write(outF)
		elif i <= 8:
			codeByte = CodeByte(cls._kCodecID, 0x8 | i)
			data = self._kBIStructs.pack(codeByte.asByte(), value)
			outF.write(data)
		else:
			CodeByte(UIntCodec._kCodecID, 0xF).write(outF)
			self._writeBigInt(i, outF)
	def encodeData(self, outF):
		i = self._dataEncoding()
		if i < 0:
			outF.write(self._kBIStructs[3].pack(b"\xF7", self.value))
		elif i <= 8:
			v = self.value | (1 << i) - 1 << (8 << i) - i
			outF.write(self._kIStructs[i].pack(v))
		else:
			outF.write(b"\xFF")
			self._writeBigInt(i, outF)
	def _dataEncoding(self):
		for i in range(4):
			if 1 << (8 << i) - i - 1 > self.value:
				return i
		if self.value < 1 << 64:
			return -1
		i = 4
		while 1 << (8 << i) <= self.value:
			i += 1
		return self._numBytes(i)
	def _objEncoding(self):
		if self.value < 8:
			return -1
		i = 0
		while 1 << (8 << i) <= self.value:
			i += 1
		return self._numBytes(i)
	def _numBytes(self, i):
		iB = 1 << i
		iA = iB >> 1
		iM = iA + iB >> 1
		while iM > iA:
			if 1 << (iM << 3) <= self.value:
				iA = iM
			else:
				iB = iM
			iM = iA + iB >> 1
		return iA
	def _writeBigInt(self, nBytes, outF):
		type(self)(nBytes - 9).encodeData(outF)
		for i in range(1 << (nBytes - 1 << 3), -8, -8):
			outF.write(bytes([self.value >> i & 0xFF]))

class SIntCodec(Codec):
	_kCodecID = Codec._kSIntID
	
	@classmethod
	def _Init(cls):
		for typ in (int, SInt):
			cls._gTypeCodec[typ] = cls
		cls._gIDCodec[cls._kCodecID] = cls
class UIntCodec(Codec):
	_kCodecID = Codec._kUIntID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[UInt] = cls
		cls._gIDCodec[cls._kCodecID] = cls
