from .ioutil import MustRead

class CodeByte:
	"""
	When encoding objects in BinON, the first byte is a special code byte
	that is split into two fields. The most-significant 4 bits are the codec
	ID, which basically corresponds to the data data type of the encoded value.
	The least-significant 4 bits are a codec-specific data field. What is
	actually stored in these bits varies. In some cases, they may store the
	object data in entirety, thereby eliminating the need for any further data
	bytes. For example, a bool would simply store a 0 or 1 in its data field.
	
	Attributes:
		codecID (int): e.g. float codec ID = 4
		data (int): data value in range [0x0,0xf]
	"""
	
	@classmethod
	def Read(cls, inF):
		"""
		Reads 1 byte from an input stream and formats it as a code byte
		
		Args:
			inF (file object): a binary input stream
		
		Returns:
			CodeByte
		
		Raises:
			ioutil.EndOfFile, Exception: see ioutil.MustRead()
		"""
		byte = MustRead(inF, 1)[0]
		return cls(byte >> 4 & 0x0F, byte & 0x0F)
	def __init__(self, codecID, data=0x0):
		self.codecID = codecID
		self.data = data
	def __repr__(self):
		return f"CodeByte(codecID = {self.codecID}, data = {self.data})"
	def asByte(self):
		"""
		Returns:
			int: an integer in range [0x00,0xff] representing the code byte
		"""
		return self.codecID << 4 & 0xF0 | self.data & 0x0F
	def write(self, outF):
		"""
		Writes the code byte to an output stream.
		
		Args:
			outF (file object): an output file object
		
		Raises:
			Exception: if outF.write() raises one
				This would tan OSError for typical file objects.
		"""
		outF.write(bytes([self.asByte()]))
	def raiseParseErr(self):
		"""
		Called by Codec Decode...() methods when the code byte makes no sense.
		"""
		raise Codec.ParseErr(f"failed to parse BinON {self}")

class Codec:
	"""
	This class encodes/decodes to/from binary file objects in the BinON format.
	
	You need never instantiate it, since all of its methods are class methods.
	You may call Codec.Encode() and Codec.Decode() directly on the Codec base
	class.
	
	WARNING:
		All other methods must be called on a subclass of Codec instead.
		For example, you may call FloatCodec.EncodeObj() but NEVER
		Codec.EncodeObj().
	"""
	
	class ParseErr(RuntimeError):
		pass
	class TypeErr(ValueError):
		def __init__(self, typ):
			super().__init__(f"failed to parse BinON {typ}")
	
	_kNullID = 0
	_kBoolID = 1
	_kSIntID = 2
	_kUIntID = 3
	_kFloatID = 4
	_kBytesID = 5
	_kStrID = 6
	_kSListID = 8
	_kGListID = 9
	_kSDictID = 10
	_kSKDictID = 11
	_kGDictID = 12
	
	_gTypeCodec = {}
	_gIDCodec = {}
	
	@classmethod
	def Encode(cls, value, outF):
		"""
		Serializes an input value and writes it to an output stream in
		BinON format.
		
		Args:
			value (object): a value of type compatible with BinON
			outF (file object): a binary output stream
				This may be an instance of any class implementing a
				write() method that accepts bytes-like objects.
		
		Raises:
			Codec.TypeErr: if type(value) is incompatible with BinON.
			Exception: any other I/O exception raised by outF.write().
				This would typically be an OSError for a file object.
		"""
		typ = type(value)
		try:
			codec = cls._gTypeCodec[typ]
		except KeyError:
			raise cls.TypeErr(typ)
		codec.EncodeObj(value, outF)
	@classmethod
	def Decode(cls, inF):
		"""
		Reads a BinON-encoded object from an input stream and returns
		the original value.
		
		Args:
			inF (file object): a binary input stream
				Decode() calls ioutil.MustRead() to read from this stream.
		
		Returns:
			object: the decoded value
		
		Raises:
			Codec.ParseErr: if decoded data in unrecognized format
			ioutil.EndOfFile, Exception: see ioutil.MustRead()
		"""
		codeByte = CodeByte.Read(inF)
		try:
			codec = cls._gIDCodec[codeByte.codecID]
		except KeyError:
			raise CodeByte.ParseErr(codeByte)
		return codec.DecodeObj(inF, codeByte)
	
	@classmethod
	def EncodeObj(cls, value, outF):
		"""
		WARNING: Do not call this method directly on the Codec class
		Call it on a subclass of Codec instead.
		
		If you already know that the value you are encoding is a simple key
		dictionary, for example, you can call SKDictCodec.EncodeObj() to save a
		modicum of overhead over Codec.Encode(), since the latter would need to
		look up the value's data type.
		
		Args:
			value (object): a value of type matching what the codec expects
			outF (file object): a binary output stream
		"""
		CodeByte(cls._kCodecID).write(outF)
	@classmethod
	def EncodeData(cls, value, outF):
		"""
		WARNING: Do not call this method directly on the Codec class
		Call it on a subclass of Codec instead.
		
		EncodeData() differs from EncodeObj() in that it need not encode a codec
		ID together with the object data. For example, if you wanted to use the
		variable length integer encoding in some context outside of BinON, you
		could go:
		
			>>> outF = io.BytesIO()
			>>> SIntCodec.EncodeData(1000, outF)
			>>> outF.getvalue()
			b'\xe8\x83'
			>>> inF = io.BytesIO(outF.getValue())
			>>> SIntCodec.DecodeData(inF)
			1000
		
		You need to know enough to call the right codec's DecodeData() method,
		since Codec.Decode() will not know what to do with this without the
		coded ID.
		
		Args:
			value (object): a value of type matching what the codec expects
			outF (file object): a binary output stream
		"""
		cls.EncodeObj(value, outF)
	@classmethod
	def EncodeDataList(cls, seq, outF):
		"""
		WARNING: Do not call this method directly on the Codec class
		Call it on a subclass of Codec instead.
		
		In most cases, EncodeDataLst() does nothing more than call EncodeData()
		on each item in the loop. It does not even encode the length of the
		list. The method is, however, overload by the BoolCodec to pack 8
		bool values to a byte.
		
		Args:
			seq (iterable of object): objects of type matching codec
			outF (file object): a binary output stream
		"""
		for value in seq:
			cls.EncodeData(value, outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		"""
		WARNING: Do not call this method directly on the Codec class
		Call it on a subclass of Codec instead.
		
		Decodes what was encoded by EncodeObj().
		
		Input:
			inF (file object): a binary input stream
			codeByte (CodeByte or None, optional): first encoded byte
				The Decode() method needs to read the first byte to determine
				which codec was used to encode the object. If you are calling
				DecodeObj() directly on a subclass of Codec, this is not
				necessary and you can let this argument default to None.
		
		Returns:
			object: the encoded value
		"""
		cls._CheckCodeByte(codeByte, inF)
		return None
	@classmethod
	def DecodeData(cls, inF):
		"""
		WARNING: Do not call this method directly on the Codec class
		Call it on a subclass of Codec instead.
		
		Decodes what was encoded by EncodeData().
		
		Input:
			inF (file object): a binary input stream
		
		Returns:
			object: the encoded value
		"""
		return cls.DecodeObj(CodeByte.Read(inF), inF)
	@classmethod
	def DecodeDataList(cls, inF, size):
		"""
		WARNING: Do not call this method directly on the Codec class
		Call it on a subclass of Codec instead.
		
		Decodes what was encoded by EncodeDataList().
		
		Input:
			inF (file object): a binary input stream
			size (int): length of the original sequence
		
		Returns:
			list: the original sequence as a list object
		"""
		return [cls.DecodeData(inF) for i in range(size)]
	
	@staticmethod
	def _CheckCodeByte(codeByte, inF):
		if codeByte is None:
			codeByte = CodeByte.Read(inF)
		return codeByte
