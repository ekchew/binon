from .binonobj import BinONObj
from .listobj import ListObj, SList


class DictObj(BinONObj):
    """
    Dictionary objects essentially encode as a pair of list objects: one for
    the keys and another for the values. But since the lists would always be of
    the same length, this length is only encoded once ahead of the keys list.
    """
    kBaseType = 9

    @classmethod
    def DecodeData(cls, inF, asObj=False):
        keys = ListObj.DecodeData(inF, asObj)
        values = ListObj.DecodeElems(inF, len(keys), asObj)
        if asObj:
            keys = keys.value
            values = values.value
        dct = {k: v for k, v in zip(keys, values)}
        return cls(dct) if asObj else dct

    @classmethod
    def _OptimalObj(cls, value, inList):
        if cls._OptimizeLog():
            with cls._OptimizeLogIndent() as indent:
                print(
                    indent + "Assessing keys for optimization...",
                    file=cls._OptimizeLog()
                )
                keys = ListObj._OptimalObj(value.keys(), inList=False)
                print(
                    indent + "Assessing values for optimization...",
                    file=cls._OptimizeLog()
                )
                vals = ListObj._OptimalObj(value.values(), inList=False)
            if type(keys) is SList:
                if type(vals) is SList:
                    print(
                        cls._IndentStr() + "Optimized to: SDict of",
                        keys.elemCls.__name__, ":", vals.elemCls.__name__,
                        file=cls._OptimizeLog()
                    )
                    return SDict(
                        value, keyCls=keys.elemCls, valCls=vals.elemCls
                    )
                print(
                    cls._IndentStr() + "Optimized to: SKList of",
                    keys.elemCls.__name__, file=cls._OptimizeLog()
                )
                return SKDict(value, keyCls=keys.elemCls)
        else:
            keys = ListObj._OptimalObj(value.keys(), inList=False)
            vals = ListObj._OptimalObj(value.values(), inList=False)
            if type(keys) is SList:
                if type(vals) is SList:
                    return SDict(
                        value, keyCls=keys.elemCls, valCls=vals.elemCls
                    )
                return SKDict(value, keyCls=keys.elemCls)
        return cls(value)

    def __init__(self, value=None):
        super().__init__(value or {})

    def encodeData(self, outF):
        ListObj(self.value.keys()).encodeData(outF)
        ListObj(self.value.values()).encodeElems(outF)


class SKDict(DictObj):
    """
    A simple key dictionary is one in which all the keys must be of the same
    data type. They can then be encoded internally as an SList (rather than a
    more general ListObj).
    """
    kSubtype = 2

    def __init__(self, value, keyCls=None):
        super().__init__(value)
        if keyCls:
            self.keyCls = keyCls
        else:
            try:
                self.keyCls = type(
                    self.BaseObj(next(iter(self.value.keys())))
                )
            except StopIteration:
                raise ValueError("could not determine SKDict key type")

    def __repr__(self):
        return f"SKDict({self.value!r}, keyCls={self.keyCls.__name__})"

    @classmethod
    def DecodeData(cls, inF, asObj=False):
        keys = SList.DecodeData(inF, asObj)
        values = ListObj.DecodeElems(inF, len(keys), asObj)
        if asObj:
            keys = keys.value
            values = values.value
        dct = {k: v for k, v in zip(keys, values)}
        return cls(dct) if asObj else dct

    def encodeData(self, outF):
        SList(self.value.keys(), elemCls=self.keyCls).encodeData(outF)
        ListObj(self.value.values()).encodeElems(outF)


class SDict(DictObj):
    """
    In a simple dictionary, the keys must all be of one data type and the
    values must also be of one data type (not necessary the same as the keys').
    This allows both the keys and values to be encoded internally as SList
    data.
    """
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
            self.keyCls = keyCls if keyCls else type(self.BaseObj(key))
            self.valCls = valCls if valCls else type(self.BaseObj(val))

    def __repr__(self):
        return "SKDict({!r}, keyCls={}, valCls={})".format(
            self.value, self.keyCls.__name__, self.valCls.__name__
        )

    @classmethod
    def DecodeData(cls, inF, asObj=False):
        keys = SList.DecodeData(inF, asObj)
        values = SList.DecodeElems(inF, len(keys), asObj)
        if asObj:
            keys = keys.value
            values = values.value
        dct = {k: v for k, v in zip(keys, values)}
        return cls(dct) if asObj else dct

    def encodeData(self, outF):
        SList(self.value.keys(), elemCls=self.keyCls).encodeData(outF)
        SList(self.value.values(), elemCls=self.valCls).encodeElems(outF)


BinONObj._InitSubcls(DictObj, [SKDict, SDict], [dict])
