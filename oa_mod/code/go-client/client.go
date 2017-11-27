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
	client = g.NewOatotClient(conn)
}

//export GOaChangeGameStage
func GOaChangeGameStage(newStage C.gameStage_t) {
}

//export GOaCloseBids
func GOaCloseBids(winner *C.char) {
	winnerStr := C.GoString(winner)
	_, err := client.OaCloseBids(
		context.Background(),
		&g.OaCloseBidsRequest{
			Winner: &winnerStr,
		},
	)
	if err != nil {
		log.Fatalf("OaCloseBids: %v", err)
	}
}

//export GOaCloseBidsByIncident
func GOaCloseBidsByIncident() {
	_, err := client.OaCloseBidsByIncident(
		context.Background(),
		&g.OaCloseBidsByIncidentRequest{},
	)
	if err != nil {
		log.Fatalf("OaCloseBidsByIncident: %v", err)
	}
}

//export GOaIsNew
func GOaIsNew(clGuid *C.char) C.int {
	clGuidStr := C.GoString(clGuid)
	res, err := client.OaIsNew(
		context.Background(),
		&g.OaIsNewRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
		},
	)
	if err != nil {
		log.Fatalf("OaIsNew: %v", err)
	}
	if *res.Result {
		return C.int(1)
	} else {
		return C.int(0)
	}
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
func GOaTransferMoney(clGuid *C.char, amount C.int, currency *C.char) {
	clGuidStr := C.GoString(clGuid)
	currencyStr := C.GoString(currency)
	amountVal := uint64(amount)
	_, err := client.OaTransferMoney(
		context.Background(),
		&g.OaTransferMoneyRequest{
			Amount:   &amountVal,
			OaAuth:   &g.OaAuth{ClGuid: &clGuidStr},
			Currency: &currencyStr,
		},
	)
	if err != nil {
		log.Fatalf("OaTransferMoney: %v", err)
	}
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
func GOaDiscardBet(clGuid *C.char, betId C.int) {
	clGuidStr := C.GoString(clGuid)
	betIdVal := uint64(betId)
	_, err := client.OaDiscardBet(
		context.Background(),
		&g.OaDiscardBetRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
			BetId:  &betIdVal,
		},
	)
	if err != nil {
		log.Fatalf("OaDiscardBet: %v", err)
	}
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
