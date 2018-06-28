class FileResult():
    def __init__(self, filepath, filehash, size, date):
        self.filepath = filepath
        self.hash = filehash
        self.size = size
        self.modified_date = date
    # end init

    def __eq__(self, other):
        return self.hash == other.hash\
         and self.filepath == other.filepath\
         and self.size == other.size\
         and self.modified_date == other.modified_date

#end FileResult

class ScanResult():
    def __init__(self):
        self.parent_directories = {}
        self.filemap = {}
    # end init
#end FileResult