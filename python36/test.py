from binon.binonobj import BinONObj
from binon.ioutil import HexDump
from io import BytesIO
import sys, traceback

try:
	outF = BytesIO()
	BinONObj.Encode(1, outF)
	HexDump(outF.getvalue())
except Exception as err:
	print("ERROR:", err, file=sys.stderr)
	traceback.print_exc(file=sys.stderr)
