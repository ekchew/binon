from .binonobj import BinONObj
from .codebyte import CodeByte
from .ioutil import MustRead

class IntObj(BinONObj):
	kBaseType = 2
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		data = bytearray(MustRead(inF, 1))
		byte0 = data[0]
		if (byte0 & 0x80) == 0:
			m = 0x7f
			n = 1
		elif (byte0 & 0x40) == 0:
			m = 0x3fff
			n = 2
		elif (byte0 & 0x20) == 0:
			m = 0x1fffffff
			n = 4
		elif (byte0 & 0x10) == 0:
			m = 0x0fffffffffffffff
			n = 8
		elif (byte0 & 0x01) == 0:
			m = 0xffffffffffffffff
			n = 9
		else:
			n = self.DecodeData(inF)
			m = (1 << n + 3) - 1
			n += 1
		if n > 1:
			data.extend(MustRead(inF, n-1))
		v = int.from_bytes(data, cls.kEndian) & m
		m += 1
		if v >= m >> 1:
			v -= m
		return cls(v) if asObj else v
	
	@classmethod
	def _AsObj(cls, value, specialize):
		return UInt(value) if specialize and value >= 0 else IntObj(value)

	def encodeData(self, outF):
		if -0x40 <= self.value < 0x40:
			m = 0x00
			n = 1
		elif -0x2000 <= self.value < 0x2000:
			m = 0x8000
			n = 2
		elif -0x10000000 <= self.value < 0x10000000:
			m = 0xC0000000
			n = 4
		elif -0x0800000000000000 <= self.value < 0x0800000000000000:
			m = 0xE000000000000000
			n = 8
		elif -0x8000000000000000 <= self.value < 0x8000000000000000:
			m = 0xF00000000000000000
			n = 9
		else:
			m = 0
			v = -self.value - 1 if self.value < 0 else self.value
			n = self.value.bit_length() + 8 >> 3
			outF.write(b"\xf1")
			self.encodeData(n)
		v = self.value & (1 << n + 3) - 1 | m
		outF.write(v.to_bytes(n, self.kEndian))

class UInt(IntObj):
	kSubtype = 2
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		data = bytearray(MustRead(inF, 1))
		byte0 = data[0]
		if (byte0 & 0x80) == 0:
			m = 0x7f
			n = 1
		elif (byte0 & 0x40) == 0:
			m = 0x3fff
			n = 2
		elif (byte0 & 0x20) == 0:
			m = 0x1fffffff
			n = 4
		elif (byte0 & 0x10) == 0:
			m = 0x0fffffffffffffff
			n = 8
		elif (byte0 & 0x01) == 0:
			m = 0xffffffffffffffff
			n = 9
		else:
			n = self.DecodeData(inF)
			m = (1 << n + 3) - 1
			n += 1
		if n > 1:
			data.extend(MustRead(inF, n-1))
		v = int.from_bytes(data, cls.kEndian) & m
		return cls(v) if asObj else v
	
	def encodeData(self, outF):
		if self.value < 0x80:
			m = 0x00
			n = 1
		elif self.value < 0x4000:
			m = 0x8000
			n = 2
		elif self.value < 0x20000000:
			m = 0xC0000000
			n = 4
		elif self.value < 0x1000000000000000:
			m = 0xE000000000000000
			n = 8
		elif self.value < 0x10000000000000000:
			m = 0xF00000000000000000
			n = 9
		else:
			m = 0
			n = self.value.bit_length() + 7 >> 3
			outF.write(b"\xf1")
			self.encodeData(n)
		outF.write((m | self.value).to_bytes(n, self.kEndian))

def _Init():
	cb = CodeByte(baseType=IntObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = IntObj
	cb.subtype = UInt.kSubtype
	BinONObj._gCodeObjCls[cb] = UInt
	for typ in (int, IntObj, UInt):
		BinONObj._gTypeBaseCls[typ] = IntObj

_Init()