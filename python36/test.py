from binon.binonobj import BinONObj
from binon.ioutil import HexDump
from io import BytesIO
import sys, traceback

try:
	outF = BytesIO()
	for i in range(100):
		outF = BytesIO()
		v0 = 1.0 / i if i > 0 else 0.0
		BinONObj.Encode(v0, outF, specialize=True)
		print(i, v0)
		HexDump(outF.getvalue())
		v1 = BinONObj.Decode(BytesIO(outF.getvalue()))
		if v1 != v0:
			print(f"Expected {v0} but got {v1}")
except Exception as err:
	print("ERROR:", err, file=sys.stderr)
	traceback.print_exc(file=sys.stderr)
