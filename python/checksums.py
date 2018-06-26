import hashlib, zlib

MD5 = hashlib.md5
SHA1 = hashlib.sha1
SHA2 = hashlib.sha256

# We need to wrap the short hashes in a similar interface

class Adler32:
    __implementation = zlib.adler32

    def __init__(self):
        self.state = 0

    def update(self, new_bytes):
        self.state = Adler32.__implementation(new_bytes, self.state) & 0xffffffff
    
    def hexdigest(self):
        return hex(self.state)[2:]
#end class Adler32

class CRC32:
    __implementation = zlib.crc32

    def __init__(self):
        self.state = 0

    def update(self, new_bytes):
        self.state = CRC32.__implementation(new_bytes, self.state) & 0xffffffff
    
    def hexdigest(self):
        return hex(self.state)[2:]