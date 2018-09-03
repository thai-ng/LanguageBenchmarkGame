package main

import "fmt"

// Line used to format and sort the output
type Line struct {
	operation rune
	file      FileResult
}

func (l Line) String() string {
	return fmt.Sprintf("%s %s", string(l.operation), l.file)
}

// Check to see if err is null, panic otherwise
func Check(err error) {
	if err != nil {
		panic(err)
	}
}
