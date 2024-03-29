from .binonobj import BinONObj
from .ioutil import MustRead


class BoolObj(BinONObj):
    kBaseType = 1

    @classmethod
    def _OptimalObj(cls, value, inList):
        makeTrueObj = value and not inList
        if cls._OptimizeLog() and makeTrueObj:
            print(
                cls._IndentStr() + "Optimized to: TrueObj",
                file=cls._OptimizeLog()
            )
        return TrueObj(value) if makeTrueObj else cls(value)

    @classmethod
    def DecodeData(cls, inF, asObj=False):
        v = bool(MustRead(inF, 1)[0])
        return cls(v) if asObj else v

    def __init__(self, value=False):
        super().__init__(value)

    def encode(self, outF):
        """
        Though not strictly necessary, BoolObj's encode() method has been
        overridden to write a more condensed TrueObj() where applicable, even
        without the optimize option set True in BinONObj.Encode().
        """
        if self.value and not isinstance(self, TrueObj):
            TrueObj(self.value).encode(outF)
        else:
            super().encode(outF)

    def encodeData(self, outF):
        outF.write(b"\1" if self.value else b"\0")


class TrueObj(BoolObj):
    kSubtype = 2

    @classmethod
    def DecodeData(cls, inF, asObj=False):
        return cls(True) if asObj else True

    def __init__(self, ignored=None):
        super().__init__(True)

    def encodeData(self, outF): pass


BinONObj._InitSubcls(BoolObj, [TrueObj], [bool])
