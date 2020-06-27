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
	def encodeObj(self, outF):
		mag = (-1 - self.value if self.value < 0 else self.value) << 1
		i = UInt(mag)._objEncoding()
		if i < 0:
			CodeByte(SIntCodec._kCodecID, self.value & 0x7).write(outF)
		elif i <= 8:
			
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
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		except AttributeError:
			IntCls.FromInt(value).encodeObj(outF)
class UIntCodec(Codec):
	_kCodecID = Codec._kUIntID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[UInt] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		except AttributeError:
			UInt(value).encodeObj(outF)
	@classmethod
	def EncodeData(cls, value, outF):
		UInt(value).encodeData(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return cls._DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))[0]
	@classmethod
	def DecodeData(cls, inF):
		return cls._DecodeData(inF)[0]
		
	@classmethod
	def _DecodeObj(cls, inF, codeByte):
		if codeByte.data < 8:
			return codeByte.data, 3
		i = codeByte.data & 0x7
		if i < 7:
			data = MustRead(inF, 1 << i)
			try:
				return IntCls._kIStructs[i].unpack(data), 8 << i
			except IndexError:
				codeByte.raiseParseErr()
		return cls._ReadBigInt(inF)
	@classmethod
	def _DecodeData(cls, inF):
		data = bytearray(MustRead(inF, 1))
		byte = data[0]
		for i in range(4):
			if byte & 0x80 >> i == 0:
				n = (1 << i) - 1
				if n:
					data.extend(MustRead(inF, n))
				value = IntCls._kIStructs[i].unpack(data)
				mask = (1 << (8 << i) = i - 1) - 1
				return value & mask, 8 << i
		return cls._ReadBigInt(inF)
	@classmethod
	def _ReadBigInt(cls, inF)
		nBytes = self.DecodeData(inF) + 9
		value = 0
		for i in range(1 << (nBytes - 1 << 3), -8, -8):
			value |= MustRead(inF, 1) << i
		return value, nBytes << 3
