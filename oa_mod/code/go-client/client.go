package main

import (
	g "github.com/oatot/oatot/generated"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"log"
	"reflect"
	"unsafe"
)

// #include "../game/g_local.h"
// #include "../game/bg_public.h"
import "C"

var (
	grpcAddr = "127.0.0.1:13283"
	client   g.OatotClient
)

func StringToC(str string, cStr *C.char) {
	size := len(str)
	var slice []C.char
	sliceHeader := (*reflect.SliceHeader)((unsafe.Pointer(&slice)))
	sliceHeader.Cap = maxCStrLen
	sliceHeader.Len = maxCStrLen
	sliceHeader.Data = uintptr(unsafe.Pointer(cStr))
	for i := 0; i < size; i++ {
		slice[i] = C.char(str[i])
	}
}

func CBidFromGo(in *g.Bid, out *C.bid_t) {
	timeStr := (*in.OpenTime).String()
	out.amount = C.int(*in.Amount)
	out.bet_ID = C.int(*in.BetId)
	StringToC(*in.Horse, &(out.horse[0]))
	StringToC(*in.Currency, &(out.currency[0]))
	StringToC(timeStr, &(out.open_time[0]))
}

func CFullbidFromGo(in *g.Bid, out *C.fullbid_t) {
	timeStr := (*in.CloseTime).String()
	CBidFromGo(in, &out.open_bid)
	out.prize = C.int(*in.Prize)
	StringToC(*in.Winner, &(out.winner[0]))
	StringToC(timeStr, &(out.close_time[0]))
}

func CCurrencySummaryFromGo(in *g.CurrencySummary) C.currencySummary_t {
	return C.currencySummary_t{
		total_bet:   C.int(*in.TotalBet),
		total_prize: C.int(*in.TotalPrize),
		total_lost:  C.int(*in.TotalLost),
		bets_won:    C.int(*in.BetsWon),
		bets_lost:   C.int(*in.BetsLost),
	}
}

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
