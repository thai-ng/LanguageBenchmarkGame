package main

import (
	"fmt"
	"hash"
	"io"
	"os"
	"sort"
	"strings"
	"time"

	set "github.com/deckarep/golang-set"
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
	defer file.Close()

	_, err = io.Copy(checksumFunction, file)
	Check(err)

	return FileResult{
		filepath:     canonicalPath,
		hash:         fmt.Sprintf("%x", checksumFunction.Sum(nil)),
		size:         fileInfo.Size(),
		timeModified: fileInfo.ModTime(),
	}
}

func populateStringSet(results map[string]FileResult) set.Set {
	paths := set.NewSet()
	for path := range results {
		paths.Add(path)
	}
	return paths
}

func generatePatch(src, target map[string]FileResult, srcPaths, targetPaths, unchanged, conflicts set.Set) map[rune][]FileResult {
	patch := map[rune][]FileResult{}

	additions := srcPaths.Difference(targetPaths)
	patch['+'] = make([]FileResult, 0, additions.Cardinality())
	additions.Each(func(entry interface{}) bool {
		path := entry.(string)
		patch['+'] = append(patch['+'], src[path])
		return false
	})

	patch['='] = make([]FileResult, 0, unchanged.Cardinality())
	unchanged.Each(func(entry interface{}) bool {
		path := entry.(string)
		patch['='] = append(patch['='], target[path])
		return false
	})

	patch['!'] = make([]FileResult, 0, conflicts.Cardinality())
	conflicts.Each(func(entry interface{}) bool {
		path := entry.(string)
		patch['!'] = append(patch['!'], target[path])
		return false
	})

	return patch
}

func reconcile(resultsA, resultsB map[string]FileResult) ReconcileResult {
	pathsA := populateStringSet(resultsA)
	pathsB := populateStringSet(resultsB)

	suspectedConflicts := pathsA.Intersect(pathsB)
	unchangedPaths := set.NewSet()

	suspectedConflicts.Each(func(entry interface{}) bool {
		path := entry.(string)
		if resultsA[path].Equal(resultsB[path]) {
			unchangedPaths.Add(path)
		}

		return false
	})

	conflicts := suspectedConflicts.Difference(unchangedPaths)

	patchResultA := generatePatch(resultsB, resultsA, pathsB, pathsA, unchangedPaths, conflicts)
	patchResultB := generatePatch(resultsA, resultsB, pathsA, pathsB, unchangedPaths, conflicts)

	return ReconcileResult{
		patchA: patchResultA,
		patchB: patchResultB,
	}
}

func writeResult(changes ReconcileResult, args ArgumentHolder) {
	filename := "reference.patch"

	if _, err := os.Stat(filename); err == nil {
		Check(os.Remove(filename))
	}

	outFile, err := os.OpenFile(filename, os.O_WRONLY|os.O_CREATE, 0777)
	Check(err)

	writeChanA := make(chan string)
	writeChanB := make(chan string)

	go writePatchResult(writeChanA, *args.directoryA, changes.patchA, *args.ignoreUnchanged)
	go writePatchResult(writeChanB, *args.directoryB, changes.patchB, *args.ignoreUnchanged)

	outFile.WriteString(fmt.Sprintf("# Results for %s\n", time.Now().Format("2006-01-02 15:04:05.000")))
	outFile.WriteString(fmt.Sprintf("# Reconciled '%s' and '%s'\n", *args.directoryA, *args.directoryB))
	outFile.WriteString(<-writeChanA)
	outFile.WriteString("\n")
	outFile.WriteString(<-writeChanB)
	outFile.WriteString("\n")
}

func writePatchResult(outChannel chan string, directory string, result map[rune][]FileResult, ignoreUnchanged bool) {
	builder := strings.Builder{}
	builder.WriteString(directory)
	builder.WriteString("\n")

	// flatten and sort before writing
	lines := make([]Line, 0)
	for op, paths := range result {
		if op == '=' && ignoreUnchanged {
			continue
		}

		for _, entry := range paths {
			lines = append(lines, Line{operation: op, file: entry})
		}
	}

	sort.Slice(lines, func(i, j int) bool {
		// Predicate for sorting
		return lines[i].file.filepath < lines[j].file.filepath
	})

	for _, line := range lines {
		builder.WriteString(line.String())
		builder.WriteString("\n")
	}

	outChannel <- builder.String()
}

func main() {
	args := setupArguments()
	fmt.Printf("Starting diff of '%s' and '%s' (%s)\n", *args.directoryA, *args.directoryB, args.checksumName)
	fmt.Printf("Starting at %s\n", time.Now().Format("2006-01-02 15:04:05.000"))

	scanChannelA := make(chan map[string]FileResult)
	scanChannelB := make(chan map[string]FileResult)

	go func() { scanChannelA <- scanDirectory(*args.directoryA, args) }()
	go func() { scanChannelB <- scanDirectory(*args.directoryB, args) }()

	resultA := <-scanChannelA
	resultB := <-scanChannelB

	changes := reconcile(resultA, resultB)
	writeResult(changes, args)

	fmt.Printf("Finished at %s\n", time.Now().Format("2006-01-02 15:04:05.000"))
}
