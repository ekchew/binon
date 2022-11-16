from .binonobj import BinONObj


class NullObj(BinONObj):
    kBaseType = 0


BinONObj._InitSubcls(NullObj, [], [type(None)])
