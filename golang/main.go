package main

import "fmt"
import kingpin "gopkg.in/alecthomas/kingpin.v2"

var (
	directoryA      = kingpin.Arg("Directory A", "Directory to parse").Required().String()
	directoryB      = kingpin.Arg("Directory B", "Directory to parse").Required().String()
	ignoreUnchanged = kingpin.Flag("ignore-unchanged", "Ignore unchagned files in the final output").Short('u').Bool()
)

func main() {
	kingpin.Version("0.0.1")
	kingpin.Parse()
	fmt.Printf("dirA %s dirB %s\n", *directoryA, *directoryB)
	fmt.Printf("ignoring unchanged %t\n", *ignoreUnchanged)
}
