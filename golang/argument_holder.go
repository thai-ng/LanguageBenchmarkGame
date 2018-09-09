package main

import (
	"crypto/md5"
	"crypto/sha1"
	"crypto/sha256"
	"hash"
	"hash/adler32"
	"hash/crc32"
	"path/filepath"

	kingpin "gopkg.in/alecthomas/kingpin.v2"
)

// ArgumentHolder holds the arguments as parsed and determines the checksum name
type ArgumentHolder struct {
	directoryA      *string
	directoryB      *string
	ignoreUnchanged *bool

	// Set by the function below
	checksumName string
}

func (args *ArgumentHolder) validate(app *kingpin.Application, optionMap map[string]*bool) {
	chosenChecksum := ""
	for hashName, optionSelected := range optionMap {
		if !(*optionSelected) {
			continue
		}

		if chosenChecksum != "" {
			app.FatalUsage("Expected only option '%s' or '%s'\n", chosenChecksum, hashName)
		}

		chosenChecksum = hashName
	}

	if chosenChecksum == "" {
		chosenChecksum = "md5"
	}

	args.checksumName = chosenChecksum

	// Also do directory clean up
	*args.directoryA = filepath.Clean(*args.directoryA)
	*args.directoryB = filepath.Clean(*args.directoryB)
}

type checksumGenerator func() hash.Hash

func (args *ArgumentHolder) getChecksumInstance() hash.Hash {
	return checksums[args.checksumName]()
}

func getAdler32() hash.Hash {
	return adler32.New().(hash.Hash)
}

func getCRC32() hash.Hash {
	return crc32.NewIEEE().(hash.Hash)
}

var checksums = map[string]checksumGenerator{
	"md5":     md5.New,
	"sha1":    sha1.New,
	"sha256":  sha256.New,
	"adler32": getAdler32,
	"crc32":   getCRC32,
}
