from .binonobj import BinONObj
from .listobj import ListObj, SList

class DictObj(BinONObj):
	kBaseType = 9
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		keys = ListObj.DecodeData(inF, asObj)
		values = ListObj.DecodeElems(inF, len(keys), asObj)
		if asObj:
			keys = keys.value
			values = values.value
		dct = {k:v for k,v in zip(keys, values)}
		return cls(dct) if asObj else dct
	
	@classmethod
	def _AsObj(cls, value, specialize):
		pass #TODO
	
	def encodeData(self, outF):
		ListObj(self.value.keys()).encodeData(outF)
		ListObj(self.value.values()).encodeElems(outF)

class SKDict(DictObj):
	kSubtype = 2
	
	def __init__(self, value, keyCls=None):
		super().__init__(value)
		if keyCls:
			self.keyCls = keyCls
		else:
			try:
				self.keyCls = type(self.AsObj(next(iter(self.value.keys()))))
			except StopIteration:
				raise ValueError("could not determine SKDict key type")
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		keys = SList.DecodeData(inF, asObj)
		values = ListObj.DecodeElems(inF, len(keys), asObj)
		if asObj:
			keys = keys.value
			values = values.value
		dct = {k:v for k,v in zip(keys, values)}
		return cls(dct) if asObj else dct
	
	def encodeData(self, outF):
		SList(self.value.keys(), elemCls=self.keyCls).encodeData(outF)
		ListObj(self.value.values()).encodeElems(outF)

class SDict(DictObj):
	kSubtype = 3
	
	def __init__(self, value, keyCls=None, valCls=None):
		super().__init__(value)
		if keyCls and valCls:
			self.keyCls = keyCls
			self.valCls = valCls
		else:
			try:
				key, val = next(iter(self.value.items()))
			except StopIteration:
				raise ValueError("could not determine SDict key/value types")
			self.keyCls = keyCls if keyCls else type(self.AsObj(key))
			self.valCls = valCls if valCls else type(self.AsObj(val))
	
	@classmethod
	def DecodeData(cls, inF, asObj=False):
		keys = SList.DecodeData(inF, asObj)
		values = SList.DecodeElems(inF, len(keys), asObj)
		if asObj:
			keys = keys.value
			values = values.value
		dct = {k:v for k,v in zip(keys, values)}
		return cls(dct) if asObj else dct
	
	def encodeData(self, outF):
		SList(self.value.keys(), elemCls=self.keyCls).encodeData(outF)
		SList(self.value.values(), elemCls=self.valCls).encodeElems(outF)

def _Init():
	cb = CodeByte(baseType=ListObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = ListObj
	cb.subtype = SKDict.kSubtype
	BinONObj._gCodeObjCls[cb] = SKDict
	cb.subtype = SDict.kSubtype
	BinONObj._gCodeObjCls[cb] = SDict
	for typ in (dict, DictObj, SKDict, SDict):
		BinONObj._gTypeBaseCls[typ] = DictObj

_Init()
