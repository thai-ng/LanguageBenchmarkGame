package main

// Check to see if err is null, panic otherwise
func Check(err error) {
	if err != nil {
		panic(err)
	}
}
