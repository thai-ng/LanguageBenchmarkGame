package main

import (
	"fmt"
	"hash"
	"io"
	"os"
	"time"

	"github.com/karrick/godirwalk"
	kingpin "gopkg.in/alecthomas/kingpin.v2"
)

func setupArguments() ArgumentHolder {
	app := kingpin.New("Golang language benchmarking", "Go implementation of the language benchmarking trial")
	app.Version("0.0.1")

	// Make the arg holder
	argHolder := ArgumentHolder{
		directoryA:      app.Arg("Directory A", "Directory to parse").Required().String(),
		directoryB:      app.Arg("Directory B", "Directory to parse").Required().String(),
		ignoreUnchanged: app.Flag("ignore-unchanged", "Ignore unchagned files in the final output").Short('u').Bool(),
		checksumName:    "md5"}

	// Mutually exclusive flag group for hashes
	var checksums = map[string]*bool{
		"md5":    app.Flag("md5", "MD5 hash (default)").Bool(),
		"sha1":   app.Flag("sha1", "SHA1 hash").Bool(),
		"sha256": app.Flag("sha256", "SHA256 hash").Bool(),
		// adler32Hash = kingpin.Flag("adler32", "Adler 32-bit checksum").Bool()
		// crcHash     = kingpin.Flag("crc", "Cyclic Redundancy Check 32-bit checksum").Bool()
	}

	var _, err = app.Parse(os.Args[1:])
	if err != nil {
		app.FatalIfError(err, "")
	}

	argHolder.validate(app, checksums)
	return argHolder
}

func scanDirectory(directory string, args ArgumentHolder) map[string]FileResult {
	retVal := map[string]FileResult{}
	cutLength := len(directory) + 1

	godirwalk.Walk(directory, &godirwalk.Options{
		Unsorted: true,
		Callback: func(osPathname string, de *godirwalk.Dirent) error {
			if de.IsDir() {
				return nil
			}

			// use goroutines here ?
			canonicalPath := osPathname[cutLength:]
			retVal[canonicalPath] = visitFile(canonicalPath, osPathname, args.getChecksumInstance())

			return nil
		},
		ErrorCallback: func(osPathname string, err error) godirwalk.ErrorAction {
			return godirwalk.SkipNode
		},
	})

	return retVal
}

func visitFile(canonicalPath, filepath string, checksumFunction hash.Hash) FileResult {
	fileInfo, err := os.Stat(filepath)
	Check(err)
	file, err := os.Open(filepath)
	Check(err)

	_, err = io.Copy(checksumFunction, file)
	Check(err)

	return FileResult{
		filepath:     canonicalPath,
		hash:         fmt.Sprintf("%x", checksumFunction.Sum(nil)),
		size:         fileInfo.Size(),
		timeModified: fileInfo.ModTime(),
	}
}

func reconcile(resultsA, resultsB map[string]FileResult) ReconcileResult {
	// TODO
	return ReconcileResult{}
}

func writeResult(changes ReconcileResult, args ArgumentHolder) {
	// TODO
}

func main() {
	args := setupArguments()
	fmt.Printf("Starting diff of '%s' and '%s' (%s)\n", *args.directoryA, *args.directoryB, args.checksumName)
	fmt.Printf("Starting at %s\n", time.Now().Format("2006-01-02 15:04:05.000"))

	scanChannelA := make(chan map[string]FileResult)
	scanChannelB := make(chan map[string]FileResult)

	go func() {
		scanChannelA <- scanDirectory(*args.directoryA, args)
	}()

	go func() {
		scanChannelB <- scanDirectory(*args.directoryB, args)
	}()

	resultA := <-scanChannelA
	resultB := <-scanChannelB

	changes := reconcile(resultA, resultB)
	writeResult(changes, args)

	fmt.Printf("Finished at %s\n", time.Now().Format("2006-01-02 15:04:05.000"))
}
