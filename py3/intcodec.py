import sys

class IntCodec:

	@staticmethod
	def SigBits(i):
		neg = i < 0
		if neg:
			i = -1 - i
		nb = 1
		while i >= (1 << nb):
			nb <<= 1
		na = nb << 1
		while na != nb:
			nm = (na + nb) >> 1
			if i > (1 << nm):
				na = nm
			else:
				nb = nm
		return na

	def encode(self, outFile):
		pass
		