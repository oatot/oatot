package main

import (
	"log"

	g "github.com/oatot/oatot/generated"
	"google.golang.org/grpc"
)

import "C"

var (
	grpcAddr = "127.0.0.1:13283"
	client     g.OatotClient
)

//export InitializeClient
func InitializeClient() {
	// Set up a connection to the server.
	conn, err := grpc.Dial(grpcAddr, grpc.WithInsecure())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()
	client = g.NewOatotClient(conn)
}

func main() {}
