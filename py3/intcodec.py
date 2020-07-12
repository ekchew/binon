from .codec import Codec, CodeByte
from .ioutil import MustRead
from struct import Struct

class IntCls:
	"""
	Base class of UInt and SInt. You can wrap int values in the latter to save
	Codec and SIntCodec some time in determining whether your integers are
	signed or unsigned. (This is done by calling FromInt(), which returns an
	SInt if its argument is negative or a UInt otherwise.)
	
	These wrappers support the ==, <, and hash operators, meaning they can be
	sorted or placed in sets/dictionary keys. To do anything more complicated,
	you need to access the value attribute containing the int.
	
	Attributes:
		value (int): the integer value
	"""
	
	_kIStructs = [Struct(f">{c}") for c in "BHIQ"]
	
	@classmethod
	def FromInt(value):
		return SInt(value) if value < 0 else UInt(value)
	@staticmethod
	def _IStructBits(i):
		return 8 << i
	@staticmethod
	def _IStructBytes(i):
		return 1 << i
	@classmethod
	def _ObjIStructPowOf2(cls, i):
		return 1 << cls._IStructBits(i)
	@classmethod
	def _ObjIStructMask(cls, i):
		return cls._ObjIStructPowOf2(i) - 1
	@classmethod
	def _DataIStructPowOf2(cls, i):
		return 1 << cls._IStructBits(i) - i - 1
	@classmethod
	def _DataIStructMask(cls, i):
		return cls._DataIStructPowOf2(i) - 1
	@classmethod
	def _DataIStructCode(cls, i):
		return (1 << i) - 1 << (8 << i) - i
	@classmethod
	def _EncodeBuffer(cls, value, nBytes, outF):
		cls(nBytes - 9).encode(outF)
		for iBit in cls._IterBufferBitIndex(nBytes):
			outF.write(bytes([value >> iBit & 0xFF]))
	@classmethod
	def _DecodeBuffer(cls, inF):
		value = 0
		nBytes = UInt.decodeData(inF)
		for iBit in cls._IterBufferBitIndex(nBytes):
			byte = MustRead(inF, 1)[0]
			value |= byte << iBit
		return value, nBytes << 3
	@staticmethod
	def _IterBufferBitIndex(nBytes):
		return (i for i in range(nBytes - 1 << 3, -8, -8))
	
	def __init__(self, value):
		self.value = value
	def __eq__(self, rhs):
		try:
			return self.value == rhs.value
		except AttributeError:
			return self.value == rhs
	def __lt__(self, rhs):
		try:
			return self.value < rhs.value
		except AttributeError:
			return self.value < rhs
	def __hash__(self):
		return hash(self.value)
	
	def encodeObj(self, outF):
		tup = self._screenForObj()
		fns = (
			self._encodeObjByCodeByte,
			self._encodeObjByStuct,
			self._encodeObjByBuffer
		)
		fns[tup[0]](*tup[1:], outF)
	def encodeData(self, outF):
		tup = self._screenForData()
		fns = (
			self._encodeDataByStruct,
			self._encodeDataByBuffer
		)
		fns[tup[0]](*tup[1:], outF)
	
	def _encodeDataByBuffer(self, nBytes, outF):
		outF.write(b"\xff")
		self._EncodeBuffer(self.value, nBytes, outF)
class UInt(IntCls):
	@classmethod
	def DecodeObj(cls, inF, codeByte):
		return cls._DecodeObjBits(inF, codeByte)[0]
	@classmethod
	def DecodeData(cls, inF):
		return cls._DecodeDataBits(inF)[0]
	
	@classmethod
	def _DecodeObjBits(cls, inF, codeByte):
		if codeByte.data < 8:
			return codeByte.data, 3
		i = codeByte.data & 0x7
		if i < 4:
			data = MustRead(inF, cls._IStructBytes(i))
			return cls._kIStructs[i].unpack(data), cls._IStructBits(i)
		if i == 7:
			return cls._DecodeBuffer(inF)
		codeByte.raiseParseErr()
	@classmethod
	def _DecodeDataBits(cls, inF):
		data = bytearray(MustRead(inF, 1))
		byte = data[0]
		for i in range(4):
			if byte & 0x80 >> i == 0x00:
				n = (1 << i) - 1
				if n:
					data.extend(MustRead(inF, n))
				value = cls._kIStructs[i].unpack(data)
				return value & cls._DataIStructMask(i), cls._IStructBits(i)
		if (byte & 0x01) == 0x00:
			return cls._kIStructs[3].unpack(MustRead(inF, 8)), 64
		return cls._DecodeBuffer(inF)
	
	def __init__(self, value, codecID=Codec._kUIntID):
		super().__init__(value)
		self.codecID = codecID
	def _screenForObj(self):
		if self.value < 8:
			return 0,
		i = 0
		while self._ObjIStructPowOf2(i) <= self.value:
			i += 1
		return (1, i) if i < 4 else (2, self._numBytes(i))
	def _screenForData(self):
		for i in range(4):
			if self._DataIStructPowOf2(i) > self.value:
				return 0, i
		if self.value < 1 << 64:
			return 0, 4
		i = 4
		while self._ObjIStructPowOf2(i) <= self.value:
			i += 1
		return 2, self._numBytes(i)
	def _numBytes(self, i):
		iB = self._IStructBytes(i)
		iA = iB >> 1
		iM = iA + iB >> 1
		while iM > iA:
			if 1 << (iM << 3) <= self.value:
				iA = iM
			else:
				iB = iM
			iM = iA + iB >> 1
		return iB
	def _encodeObjByCodeByte(self, outF):
		CodeByte(self.codecID, self.value).write(outF)
	def _encodeObjByStruct(self, iStruct, outF):
		CodeByte(self.codecID, 0x8 | iStruct).write(outF)
		outF.write(self._kIStructs[iStruct].pack(self.value))
	def _encodeObjByBuffer(self, nBytes, outF):
		CodeByte(self.codecID, 0xF).write(outF)
		self._EncodeBuffer(self.value, nBytes, outF)
	def _encodeDataByStruct(self, iStruct, outF):
		if iStruct == 4:
			outF.write(b"\xfe")
			self._kIStructs[3].pack(self.value)
		else:
			word = self._DataIStructCode(iStruct) | value
			outf.write(self._kIStructs[iStruct].pack(word))
class SInt(IntCls):
	_kStruct64 = Struct("q")
	
	@classmethod
	def DecodeObj(cls, inF, codeByte):
		return cls.RestoreSign(*UInt._DecodeObjBits(inF, codeByte))
	@classmethod
	def DecodeData(cls, inF):
		return cls.RestoreSign(*UInt._DecodeDataBits(inF))
	@classmethod
	def RestoreSign(cls, value, nBits):
		powOf2 = 1 << nBits
		if value >= powOf2 >> 1:
			value -= powOf2
		return value
	
	def __init__(self, value):
		super().__init__(value)
	def _screenForObj(self):
		return UInt(self.magnitudeValue())._screenForObj()
	def _screenForData(self):
		return UInt(self.magnitudeValue())._screenForData()
	def _magnitudeValue(self):
		return (-1 - value if value < 0 else value) << 1
	def _encodeObjByCodeByte(self, outF):
		CodeByte(SIntCodec._kCodecID, self.value & 0x7).write(outF)
	def _encodeObjByStruct(self, iStruct, outF):
		CodeByte(SIntCodec._kCodecID, 0x8 | iStruct).write(outF)
		uInt = self.value & self._ObjIStructMask(iStruct)
		outF.write(self._kIStructs[iStruct].pack(uInt))
	def _encodeObjByBuffer(self, nBytes, outF):
		CodeByte(SIntCodec._kCodecID, 0xF).write(outF)
		self._EncodeBuffer(self.value, nBytes, outF)
	def _encodeDataByStruct(self, iStruct, outF):
		if iStruct == 4:
			outF.write(b"\xfe")
			self._kStruct64.pack(self.value)
		else:
			word = self._DataIStructCode(iStruct)
			word |= value & self._DataIStructMask(iStruct)
			outf.write(self._kIStructs[iStruct].pack(word))

class UIntCodec(Codec):
	"""
	Codec for encoding unsigned int values.
	
	WARNING:
		The Encode...() methods you see here will not check that their arguments
		indeed contain non-negative integers. If you are uncertain, pass int
		values into SIntCodec's EncodeObj...() methods insteam to perform a
		check.
	"""
	
	_kCodecID = Codec._kUIntID
	
	@classmethod
	def _Init(cls):
		cls._gTypeCodec[UInt] = cls
		cls._gIDCodec[cls._kCodecID] = cls
	@classmethod
	def EncodeObj(cls, value, outF):
		try:
			value.encodeObj(outF)
		AttributeError:
			UInt(value).encodeObj(outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		cls._EncodeObjList(lst, outF)
	@classmethod
	def EncodeData(cls, value, outF):
		try:
			value.encodeData(outF)
		AttributeError:
			UInt(value).encodeData(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return UInt.DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		return cls._DecodeObjList(inF, length, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		return UInt.DecodeData(inF)
class SIntCodec(Codec):
	"""
	Codec for encoding any integers. The EncodeObj...() methods may use an
	unsigned integer encoding in certain cases if they can get away with it. You
	can wrap your integers in SInt if you want to suppress this.
	"""
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
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		codec = cls
		if lookedUp:
			for value in lst:
				if isinstance(value, SInt) or value < 0:
					break
			else:
				codec = UIntCodec
		codec._EncodeObjList(lst, outF)
	@classmethod
	def ListElemCodec(cls, lst):
		return cls._ListElemCodec(lst)
	@classmethod
	def EncodeData(cls, value, outF):
		try:
			value.encodeData(outF)
		except AttributeError:
			SInt(value).encodeData(outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		return SInt.DecodeObj(inF, cls._CheckCodeByte(codeByte, inF))
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		return cls._DecodeObjList(inF, length, codeByte)
	@classmethod
	def DecodeData(cls, inF):
		return SInt.DecodeData(inF)
	
	@classmethod
	def _ListElemCodec(cls, lst):
		for i in lst:
			if i < 0:
				return cls
		return UIntCodec

for cls in (UIntCodec, SIntCodec):
	cls._Init()
