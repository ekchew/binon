from codebyte import CodeByte

class BinONObj:
	class ParseErr(RuntimeError): pass
	class TypeErr(ValueError): pass
	
	_gIDObjCls = {}
	_gTypeObjCls = {}
	
	@classmethod
	def AsObj(cls, value, optimal=True):
		if isinstance(value, cls):
			return value
		try:
			objCls = cls._gTypeObjCls[type(value)]
		except KeyError:
			raise cls.TypeErr("BinON cannot encode object of type:".format(
				type(value).__name__
			))
		return objCls._AsOptimalObj(value) if optimal else objCls(value)
	@classmethod
	def Encode(cls, value, outF, optimal=True):
		cls.AsObj(value, optimal).encode(outF)
	@classmethod
	def Decode(cls, inF, asObj=False):
		cb = CodeByte.Read(inF)
		try:
			objCls = cls._gIDObjCls[cb.baseType]
		except KeyError:
			raise cls.ParseErr(f"unknown BinON base type code: {cb.baseType}")
		obj = objCls._Decode(cb, inF)
		return obj if asObj else obj.asValue()
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		obj = cls()
		return obj if asObj else obj.asValue()
	
	@classmethod
	def _AsOptimalObj(cls, value):
		return cls(value)
	@classmethod
	def _Decode(cls, codeByte, inF):
		return cls.DecodeData(inF) if cls.subtype else cls()
	
	__slots__ = ["value"]
	
	def __init__(self, value=None):
		self.value = value
	def asValue(self):
		return self.value
	def encode(self, outF):
		st = CodeByte.kBaseSubtype if self.value else CodeByte.kDefaultSubtype
		CodeByte(baseType=self.kBaseType, subtype=st).write(outF)
		self.encodeData(outF)
	def encodeData(self, outF): pass
