#!/usr/bin/env python

from binon.binonobj import BinONObj
from binon.ioutil import HexDump
from io import BytesIO
import sys, traceback

try:
	outF = BytesIO()
	for i in range(100):
		outF = BytesIO()
		v0 = [bool(j & 1) for j in range(i)]
		BinONObj.Encode(v0, outF, optimize=True)
		print(i, v0)
		HexDump(outF.getvalue())
		v1 = BinONObj.Decode(BytesIO(outF.getvalue()))
		if v1 != v0:
			print(f"Expected {v0!r} but got {v1!r}")
except Exception as err:
	print("ERROR:", err, file=sys.stderr)
	traceback.print_exc(file=sys.stderr)
