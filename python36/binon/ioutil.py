import sys

class EndOfFile(RuntimeError):
	def __init__(self, inF, n):
		super().__init__(f"failed to read {n} bytes from {inF!r}")

def MustRead(inF, n):
	"""
	Normally, when reading binary data from a file object, the read() method
	returns fewer than the requested number of bytes to indicate an end of file.
	MustRead() will raise an exception in this case instead.
	
	Args:
		inF (file object): a binary input stream
			This may be an instance of any class implementing a
			read() method that accepts an integer count and returns
			a bytes-like object of up to n bytes.
		n (int): number of bytes to read from inF
	
	Returns:
		bytes-like object: the return value from inF.read()
	
	Raises:
		ioutil.EndOfFile: if inF.read() returns fewer than n bytes
		Exception: if inF.read() itself raises an exception directly
			This would typically be an OSError for file objects.
	"""
	data = inF.read(n)
	if len(data) < n:
		raise EndOfFile(inF, n)
	return data

def HexDump(data, outF=sys.stdout, **kwargs):
	addr = kwargs.pop("baseAddr", 0)
	addrFmt = kwargs.pop("addrFmt", "{:08X}:")
	byteFmt = kwargs.pop("byteFmt", "{:02X}")
	wordBytes = kwargs.pop("wordBytes", 4)
	lineWords = kwargs.pop("lineWords", 4)
	lineBytes = lineWords * wordBytes
	if kwargs:
		raise ValueError("invalid kwargs to ioutil.HexDump: {}".format(
			", ".join(f"{k} = {v!r}" for k,v in kwargs.items())
		))
	for i, byte in enumerate(data):
		iLineByte = i % lineBytes
		if iLineByte == 0:
			outF.write(addrFmt.format(addr))
		if (iLineByte % wordBytes) == 0:
			outF.write(" ")
		outF.write(byteFmt.format(byte))
		if iLineByte == lineBytes - 1:
			outF.write("\n")
			addr += lineBytes
	if i % lineBytes != lineBytes - 1:
		outF.write("\n")
