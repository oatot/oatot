package main

import (
	"flag"
	"log"
	"net"

	g "github.com/oatot/oatot/generated"
	"github.com/oatot/oatot/backend/lib"
	"github.com/oatot/oatot/backend/protobufcrpc"
)

var (
	addr = flag.String("rpc-addr", "127.0.0.1:13289", "addr")
)

func main() {
	flag.Parse()
	service, err := lib.New()
	if err != nil {
		log.Fatalf("lib.New: %v.", err)
	}
	server, err := protobufcrpc.New(&g.Oatot_serviceDesc, service)
	if err != nil {
		log.Fatalf("protobufcrpc.New: %v.", err)
	}
	ln, err := net.Listen("tcp", *addr)
	if err != nil {
		log.Fatalf("net.Listen: %v.", err)
	}
	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Fatalf("ln.Accept: %v.", err)
		}
		go func() {
			defer conn.Close()
			for {
				if err := server.Serve(conn); err != nil {
					log.Printf("server.Serve: %v.", err)
					break
				}
			}
		}()
	}
}
