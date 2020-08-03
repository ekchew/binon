from .binonobj import BinONObj

class NullObj(BinONObj):
	kBaseType = 0

def _Init():
	cb = CodeByte(baseType=NullObj.kBaseType)
	for cb.subtype in CodeByte.BaseSubtypes():
		BinONObj._gCodeObjCls[cb] = NullObj
	for typ in (None, NullObj):
		BinONObj._gTypeBaseCls[typ] = NullObj

_Init()
