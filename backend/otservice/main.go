package main

import (
	"flag"
	"log"
	"net"

	g "github.com/oatot/oatot/generated"
	"github.com/oatot/oatot/backend/lib"
	"google.golang.org/grpc"
)

var (
	grpcAddr = flag.String("grpc-addr", "127.0.0.1:13283", "grpc addr")
)

func main() {
	flag.Parse()
	server, err := lib.New()
	if err != nil {
		log.Fatalf("lib.New: %v.", err)
	}
	grpcServer := grpc.NewServer()
	g.RegisterOatotServer(grpcServer, server)
	conn, err := net.Listen("tcp", *grpcAddr)
	if err != nil {
		log.Fatalf("net.Listen: %v.", err)
	}
	if err := grpcServer.Serve(conn); err != nil {
		log.Fatalf("grpcServer.Serve: %v.", err)
	}
}
