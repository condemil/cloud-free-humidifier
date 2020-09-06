package main

import (
	"crypto/md5"
	"encoding/hex"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
)

func main() {
	var firmwareFilepath string
	var packet string

	flag.StringVar(&firmwareFilepath, "f", "", "file to search for a cloud key")
	flag.StringVar(&packet, "p", "", "hex representation of udp miio packet, starts with '2131'")
	flag.Parse()

	if packet == "" || firmwareFilepath == "" {
		flag.PrintDefaults()
		os.Exit(64) // EX_USAGE
	}

	decodedPacket, err := hex.DecodeString(packet)
	if err != nil {
		log.Fatal(err)
	}
	// fmt.Printf("%v\n", decodedPacket)
	var checksum [16]byte
	copy(checksum[:], decodedPacket[16:32])
	fmt.Printf("%v\n", checksum)

	fileData, err := ioutil.ReadFile(firmwareFilepath)
	if err != nil {
		log.Fatal(err)
	}

	i := 0
	j := 16

	for i < len(fileData) {
		if i%100000 == 0 {
			// fmt.Printf("%v ", i)
		}
		if md5.Sum(fileData[i:j]) == checksum {
			println("cloud key found: %v, i=%v", fileData[i:j], i)
		}
		i++
		j++
	}
}
