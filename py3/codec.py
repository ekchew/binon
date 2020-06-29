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
	
	Note that in addition to the object encoding and decoding methods you see
	here, there are EncodeData() and DecodeData() methods implemented in every
	subclass of Codec. EncodeData() can omit the code byte identifying the
	codec used to encode any data that follows.
	
	For example, say you wanted to use the variable-length integer encoding in
	some context outside of BinON. You could go:
	
		>>> outF = io.BytesIO()
		>>> UIntCodec.EncodeData(1000, outF)
		>>> outF.getvalue()
		b'\xe8\x83'
		>>> inF = io.BytesIO(outF.getValue())
		>>> UIntCodec.DecodeData(inF)
		1000
	
	You need to know enough to call the right DecodeData() method (in this case,
	UIntCodec.DecodeData()), of course, since this information would not be
	stored as it is by EncodeObj().
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
	_kFloat16ID = 4
	_kFloat32ID = 5
	_kFloat64ID = 6
	_kBytesID = 7
	_kStrID = 8
	_kSListID = 9
	_kGListID = 10
	_kSDictID = 11
	_kSKDictID = 12
	_kGDictID = 13
	
	_gTypeCodec = {}
	_gIDCodec = {}
		
	@classmethod
	def EncodeObj(cls, value, outF):
		"""
		This function writes a code byte followed the value data (if any).
		
		If you already know the data type of value, you can call the EncodeObj()
		implementation for the appropriate codec directly. For example, you
		could call StrCodec.EncodeObj("Hello, world!", outF). This would be
		marginally faster than Codec.EncodeObj("Hello, world!", outF), since it
		would forego the need to look up StrCodec by examining the data type of
		"Hello, world!".
		
		Args:
			value (object): a value of type matching what the codec expects
			outF (file object): a binary output stream
		"""
		typ = type(value)
		try:
			codec = cls._gTypeCodec[typ]
		except KeyError:
			raise cls.TypeErr(typ)
		codec.EncodeObj(value, outF)
	@classmethod
	def EncodeObjList(cls, lst, outF, lookedUp=False):
		"""
		This method is like EncodeObj(), except that it writes the code byte
		only once at the beginning, followed by the data for all the elements in
		a list. This implies that all the elements share the same data type.
		EncodeObjList() is called internally by SListCodec.EncodeObj() and the
		like.
		
		Note that when you call Codec.EncodeObjList() (as opposed to calling it
		on a specific subclass of Codec), the element data type is determined
		from the first element iterated out of lst. If lst is empty, the data
		type is taken to be type(None).
		
		WARNING: Note that EncodeObjList() does NOT write the length of the
		list, but you will to supply this to DecodeObjList(). If you are calling
		EncodeObjList() directly, you might want to write the length first with
		UIntCodec.EncodeData(len(myList), outF) so that you can read it back
		later, unless the list has a fixed length or its length is implied in
		some other way.
		
		Args:
			lst (list of object): objects of type matching codec
				(lst can optionally be a tuple, set, or frozenset also, but
				bear in mind that what DecodeObjList() returns will be a list
				regardless.)
			outF (file object): a binary output stream
			lookedUp (bool, optional): ignore this
				You should let this argument default to False.
				
				(It indicates whether the list element type had to be looked up,
				which can have implications for certain codecs.)
		"""
		try:
			typ = type(next(iter(lst)))
		except StopIteration:
			typ = type(None)
		try:
			codec = cls._gTypeCodec[typ]
		except KeyError:
			raise cls.TypeErr(typ)
		codec.EncodeObjList(lst, outF)
	@classmethod
	def DecodeObj(cls, inF, codeByte=None):
		"""
		Decodes what was encoded by EncodeObj().
		
		Input:
			inF (file object): a binary input stream
			codeByte (CodeByte or None, optional): ignore this
				You should let this argument default to None.
				
				(When you call Codec.DecodeObj(), it needs to read the code byte
				first to determine which specific codec was used, and then this
				gets passed into the subclass's DecodeObj() method.)
		
		Returns:
			object: the encoded value
		"""
		codeByte = cls._CheckCodeByte(codeByte, inF)
		try:
			codec = cls._gIDCodec[codeByte.codecID]
		except KeyError:
			codeByte.raiseParseErr()
		return codec.DecodeObj(inF, codeByte)
	@classmethod
	def DecodeObjList(cls, inF, length, codeByte=None):
		"""
		Decodes what was encoded by EncodeObjList().
		
		Input:
			inF (file object): a binary input stream
			length (int): length of the original sequence
			codeByte (CodeByte or None, optional): ignore this
				See DecodeObj().
		
		Returns:
			list: the original sequence as a list object
		"""
		codeByte = CodeByte.Read(inF)
		try:
			codec = cls._gIDCodec[codeByte.codecID]
		except KeyError:
			raise CodeByte.ParseErr(codeByte)
		codec.DecodeObjList(inF, length, codeByte)
	
	@staticmethod
	def _CheckCodeByte(codeByte, inF):
		if codeByte is None:
			codeByte = CodeByte.Read(inF)
		return codeByte
	@classmethod
	def _EncodeObjList(cls, lst, outF):
		CodeByte(cls._kCodecID).write(outF)
		for value in lst:
			cls.EncodeData(value, outF)
	@classmethod
	def _DecodeObjList(cls, inF, length, codeByte=None):
		cls._CheckCodeByte(codeByte, inF)
		return [cls.DecodeData(inF) for i in range(length)]
