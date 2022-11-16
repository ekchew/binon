import sys


class EndOfFile(RuntimeError):

    def __init__(self, inF, n):
        super().__init__(f"failed to read {n} bytes from {inF!r}")


def HexDump(data, outF=sys.stdout, **kwargs):
    """
    Prints a hex editor-like view of a binary data buffer, with an address
    label at the beginning of every line followed by several hexadecimal words.

    Args:
        data (bytes-like iterable): the source data to print
        outF (file object, optional): output stream, defaults to sys.stdout
        baseAddr (int, kwarg): address label starting value (defaults to 0)
            This increments by the number of bytes per line every line.
        addrFmt (str, kwarg): format() string for address label
            Defaults to "{:08x}:", which writes an 8-digit hexadecimal followed
            by a colon.
        byteFmt (str, kwarg): format() string for each data byte
            Defaults to "{:02x}", which writes a 2-digit hexadecimal.
        wordBytes (int, kwarg): number of bytes per hexadecimal word
            Defaults to 4, which means you should see a maximum 8 hexadecimal
            digits worth of binary data strung together at a time.
        wordSep (str, kwarg): character(s) to insert between words
            Defaults to " " (a single space). Note that wordSep also gets
            inserted between the address label and the first word on a line.
        lineWords (int, kwarg): number of words to print per line
            Defaults to 4. The number of bytes per line can be calculated as
            lineWords * wordBytes.
    """
    addr = kwargs.pop("baseAddr", 0)
    addrFmt = kwargs.pop("addrFmt", "{:08x}:")
    byteFmt = kwargs.pop("byteFmt", "{:02x}")
    wordBytes = kwargs.pop("wordBytes", 4)
    wordSep = kwargs.pop("wordSep", " ")
    lineWords = kwargs.pop("lineWords", 4)
    lineBytes = lineWords * wordBytes
    if kwargs:
        raise ValueError("invalid kwargs to ioutil.HexDump: {}".format(
            ", ".join(f"{k} = {v!r}" for k, v in kwargs.items())
        ))
    for i, byte in enumerate(data):
        iLineByte = i % lineBytes
        if iLineByte == 0:
            outF.write(addrFmt.format(addr))
        if (iLineByte % wordBytes) == 0:
            outF.write(wordSep)
        outF.write(byteFmt.format(byte))
        if iLineByte == lineBytes - 1:
            outF.write("\n")
            addr += lineBytes
    if i % lineBytes != lineBytes - 1:
        outF.write("\n")


def MustRead(inF, n):
    """
    Normally, when reading binary data from a file object, the read() method
    returns fewer than the requested number of bytes to indicate an end of
    file. MustRead() will raise an exception in this case instead.

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
