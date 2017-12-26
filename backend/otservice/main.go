package main

import (
	"flag"
	"io/ioutil"
	"log"
	"net"
	"os"
	"time"

	"github.com/oatot/oatot/backend/lib"
	g "github.com/oatot/oatot/generated"
	"google.golang.org/grpc"
)

var (
	grpcAddr   = flag.String("grpc-addr", "127.0.0.1:13283", "grpc addr")
	database   = flag.String("database", "", "Path to database")
	dumpPeriod = flag.Duration("dump-period", time.Second, "Database dump period")
)

func main() {
	flag.Parse()
	var server *lib.Server
	if *database != "" {
		data, err := ioutil.ReadFile(*database)
		if err != nil && !os.IsNotExist(err) {
			log.Fatalf("ioutil.ReadFile: %v.", err)
		}
		if err == nil {
			var err error
			server, err = lib.Load(data)
			if err != nil {
				log.Fatalf("lib.Load: %v.", err)
			}
		}
	}
	if server == nil {
		var err error
		server, err = lib.New()
		if err != nil {
			log.Fatalf("lib.New: %v.", err)
		}
	}
	if *database != "" {
		go func() {
			for {
				time.Sleep(*dumpPeriod)
				data, err := server.Save()
				if err != nil {
					log.Fatalf("server.Save: %v.", err)
				}
				if err := ioutil.WriteFile(*database, data, 0600); err != nil {
					log.Fatalf("ioutil.WriteFile: %v.", err)
				}
			}
		}()
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
