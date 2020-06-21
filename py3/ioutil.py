def MustRead(inFile, length):
	data = inFile.read(length)
	if len(data) < length:
		raise RuntimeError(
			"failed to read {} bytes from input stream {}".format(
				length, inFile
			)
		)
	return data
