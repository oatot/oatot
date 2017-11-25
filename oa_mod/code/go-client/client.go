package main

import (
	g "github.com/oatot/oatot/generated"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"log"
)

// #include "../game/g_local.h"
// #include "../game/bg_public.h"
import "C"

var (
	grpcAddr = "127.0.0.1:13283"
	client   g.OatotClient
)

//export GInitializeClient
func GInitializeClient() {
	// Set up a connection to the server.
	conn, err := grpc.Dial(grpcAddr, grpc.WithInsecure())
	if err != nil {
		log.Fatalf("Did not connect: %v", err)
	}
	defer conn.Close()
	client = g.NewOatotClient(conn)
}

//export GOaChangeGameStage
func GOaChangeGameStage(newStage C.gameStage_t) {
}

//export GOaCloseBids
func GOaCloseBids(winner *C.char) {
}

//export GOaCloseBidsByIncident
func GOaCloseBidsByIncident() {
}

//export GOaIsNew
func GOaIsNew(clGuid *C.char) C.int {
	return C.int(0)
}

//export GOaRegister
func GOaRegister(clGuid *C.char) {
	clGuidStr := C.GoString(clGuid)
	_, err := client.OaRegister(
		context.Background(),
		&g.OaRegisterRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
		},
	)
	if err != nil {
		log.Fatalf("OaRegister: %v", err)
	}
}

//export GOaTransferMoney
func GOaTransferMoney(clGuid *C.char, amount C.int) {
}

//export GOaActiveBidsSums
func GOaActiveBidsSums(horse *C.char) C.betSum_t {
	return C.betSum_t{1, 2}
}

//export GOaMyBalance
func GOaMyBalance(clGuid *C.char, currency *C.char) C.balance_t {
	return C.balance_t{1, 2}
}

//export GOaMyBid
func GOaMyBid(clGuid *C.char, bid C.bid_t) {
}

//export GOaDiscardBet
func GOaDiscardBet(clGuid *C.char, betId int) {
}

//export GOaMyActiveBids
func GOaMyActiveBids(clGuid *C.char, activeBids *C.bid_t) C.int {
	return 0
}

//export GOaMyPastBids
func GOaMyPastBids(clGuid *C.char, page *C.char, nextPage *C.char, pastBids *C.fullbid_t) C.int {
	return 0
}

//export GOaMyBidsSummary
func GOaMyBidsSummary(clGuid *C.char) C.bidsSummary_t {
	return C.bidsSummary_t{C.currencySummary_t{0, 0, 0, 0, 0}, C.currencySummary_t{0, 0, 0, 0, 0}}
}

func main() {}
