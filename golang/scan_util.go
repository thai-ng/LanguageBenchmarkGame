package main

import (
	"fmt"
	"time"
)

// FileResult holds the individual hashed files
type FileResult struct {
	filepath     string
	hash         string
	size         int64
	timeModified time.Time
}

func (fr FileResult) String() string {
	return fmt.Sprintf("%s (%s | %d bytes)",
		fr.filepath,
		fr.timeModified.Format("2006-01-02 15:04:05.000"),
		fr.size)
}

// ReconcileResult holds the results of the reconciliation operation
type ReconcileResult struct {
	patchA map[string][]FileResult
	patchB map[string][]FileResult
}
