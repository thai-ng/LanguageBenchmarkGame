package main

import (
	"fmt"
	"math"
	"time"
)

// FileResult holds the individual hashed files
type FileResult struct {
	filepath     string
	hash         string
	size         int64
	timeModified time.Time
}

// Equal : Checks if two file results are equal
func (fr FileResult) Equal(other FileResult) bool {
	return fr.hash == other.hash &&
		fr.size == other.size &&
		(math.Abs(fr.timeModified.Sub(other.timeModified).Seconds()) < 1) &&
		fr.filepath == fr.filepath
}

func (fr FileResult) String() string {
	return fmt.Sprintf("%s (%s | %d bytes)",
		fr.filepath,
		fr.timeModified.Format("2006-01-02 15:04:05"),
		fr.size)
}

// ReconcileResult holds the results of the reconciliation operation
type ReconcileResult struct {
	patchA map[rune][]FileResult
	patchB map[rune][]FileResult
}
