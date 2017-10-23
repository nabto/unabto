package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
)

var port = "0.0.0.0:32006"

func echo(conn net.Conn) {
	r := bufio.NewReader(conn)
	for {
		line, err := r.ReadBytes(byte('\n'))
        fmt.Println("Read line ", line)
        fmt.Println("err ", err)
		switch err {
		case nil:
			break
		case io.EOF:
            return
            break
		default:
			fmt.Println("ERROR", err)
		}
		conn.Write(line)
	}
}

func main() {
	l, err := net.Listen("tcp", port)
	if err != nil {
		fmt.Println("ERROR", err)
		os.Exit(1)
	}

	for {
		conn, err := l.Accept()
		if err != nil {
			fmt.Println("ERROR", err)
			continue
		}
        fmt.Println("received Conn");
		go echo(conn)
	}
}