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

const maxActiveBetsN = 5
const betsPerPageN = 15
const maxCStrLen = 1024
const currenciesN = 2
const horsesN = 2

var (
	grpcAddr = "127.0.0.1:13283"
	client   g.OatotClient
)

// ==================================================================
// TODO: replace complex and ugly solution of C*FromGo (or *ToC) with
// smart recursive function (using reflect), common for all types.
// Consider using `go generate` to generate the rest of this file.
// ==================================================================

// Dirty type paramertic function.
// Just in order to prevent code repetition.
func CopyGoSliceToCArray(
	cArray interface{},
	goSlice interface{},
	dataSlicePtr interface{},
	funcCopyElement interface{},
	cArraySize int,
) C.int {
	// Get values of interfaces first.
	vCArray := reflect.ValueOf(cArray)
	vGoSlice := reflect.ValueOf(goSlice)
	vDataSlicePtr := reflect.ValueOf(dataSlicePtr)
	vDataSlice := reflect.Indirect(vDataSlicePtr)
	// Len of Go slice.
	goSliceLen := vGoSlice.Len()
	if goSliceLen > cArraySize {
		log.Fatalf("Go-client: attempt to copy slice to C array of smaller size.")
	}
	// sliceHeader trick to convert the C array into a Go slice.
	sliceHeader := (*reflect.SliceHeader)(unsafe.Pointer(vDataSlicePtr.Pointer()))
	sliceHeader.Cap = cArraySize
	sliceHeader.Len = cArraySize
	sliceHeader.Data = vCArray.Pointer()
	for i := 0; i < goSliceLen; i++ {
		// Hacky reflect usage: call function to copy individual element.
		reflect.ValueOf(funcCopyElement).Call(
			[]reflect.Value{vGoSlice.Index(i), vDataSlice.Index(i).Addr()},
		)
	}
	return C.int(goSliceLen)
}

func CharToC(goChar byte, cChar *C.char) {
	*cChar = C.char(goChar)
}

func StringToC(str string, cStr *C.char) {
	str += "\x00"
	var slice []C.char
	CopyGoSliceToCArray(
		cStr,
		str,
		&slice,
		CharToC,
		maxCStrLen,
	)
}

func CBetFromGo(in *g.Bet, out *C.bet_t) {
	//TODO: implement time storage in backend.
	//timeStr := (*in.OpenTime).String()
	out.amount = C.int(*in.Amount)
	out.betID = C.int(*in.BetId)
	StringToC(*in.Horse, &(out.horse[0]))
	StringToC(*in.Currency, &(out.currency[0]))
	//StringToC(timeStr, &(out.openTime[0]))
}

func CFullbetFromGo(in *g.Bet, out *C.fullbet_t) {
	//TODO: implement time storage in backend.
	//timeStr := (*in.CloseTime).String()
	CBetFromGo(in, &out.openBet)
	out.prize = C.int(*in.Prize)
	StringToC(*in.Winner, &(out.winner[0]))
	//StringToC(timeStr, &(out.closeTime[0]))
}

func CBetSumFromGo(in *g.BetSum, out *C.betSum_t) {
	out.amount = C.int(*in.Amount)
	StringToC(*in.Currency, &(out.currency[0]))
	StringToC(*in.Horse, &(out.horse[0]))
}

func CCurrencySummaryFromGo(in *g.CurrencySummary, out *C.currencySummary_t) {
	out.totalBet = C.int(*in.TotalBet)
	out.totalPrize = C.int(*in.TotalPrize)
	out.totalLost = C.int(*in.TotalLost)
	out.betsWon = C.int(*in.BetsWon)
	out.betsLost = C.int(*in.BetsLost)
	StringToC(*in.Currency, &(out.currency[0]))
}

func CBalanceFromGo(in *g.Balance, out *C.balance_t) {
	out.freeMoney = C.int(*in.FreeMoney)
	out.moneyOnBets = C.int(*in.MoneyOnBets)
	StringToC(*in.Currency, &(out.currency[0]))
}

//export GInitializeClient
func GInitializeClient() {
	if client == nil {
		// Set up a connection to the server.
		conn, err := grpc.Dial(grpcAddr, grpc.WithInsecure())
		if err != nil {
			log.Fatalf("Did not connect: %v", err)
		}
		client = g.NewOatotClient(conn)
	}
}

//export GOaChangeGameStage
func GOaChangeGameStage(newStage C.int) {
	newStageVal := g.GameStage(newStage)
	_, err := client.OaChangeGameStage(
		context.Background(),
		&g.OaChangeGameStageRequest{
			NewStage: &newStageVal,
		},
	)
	if err != nil {
		log.Fatalf("OaChangeGameStage: %v", err)
	}
}

//export GOaCloseBets
func GOaCloseBets(winner *C.char) {
	winnerStr := C.GoString(winner)
	_, err := client.OaCloseBets(
		context.Background(),
		&g.OaCloseBetsRequest{
			Winner: &winnerStr,
		},
	)
	if err != nil {
		log.Fatalf("OaCloseBets: %v", err)
	}
}

//export GOaCloseBetsByIncident
func GOaCloseBetsByIncident() {
	_, err := client.OaCloseBetsByIncident(
		context.Background(),
		&g.OaCloseBetsByIncidentRequest{},
	)
	if err != nil {
		log.Fatalf("OaCloseBetsByIncident: %v", err)
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

//export GOaActiveBetsSums
func GOaActiveBetsSums(betSums *C.betSum_t) C.int {
	res, err := client.OaActiveBetsSums(
		context.Background(),
		&g.OaActiveBetsSumsRequest{},
	)
	if err != nil {
		log.Fatalf("OaActiveBetsSums: %v", err)
	}
	var sums []C.betSum_t
	return CopyGoSliceToCArray(
		betSums,
		res.BetSums,
		&sums,
		CBetSumFromGo,
		currenciesN * horsesN,
	)

}

//export GOaMyBalance
func GOaMyBalance(clGuid *C.char, balances *C.balance_t) C.int {
	clGuidStr := C.GoString(clGuid)
	res, err := client.OaMyBalance(
		context.Background(),
		&g.OaMyBalanceRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
		},
	)
	if err != nil {
		log.Fatalf("OaMyBalance: %v", err)
	}
	var cBalances []C.balance_t
	return CopyGoSliceToCArray(
		balances,
		res.Balances,
		&cBalances,
		CBalanceFromGo,
		currenciesN,
	)
}

//export GOaMyBet
func GOaMyBet(clGuid *C.char, bet C.bet_t) {
	clGuidStr := C.GoString(clGuid)
	horseStr := C.GoString(&(bet.horse[0]))
	currencyStr := C.GoString(&(bet.currency[0]))
	amountVal := uint64(bet.amount)
	betN := &g.Bet{
		Horse:    &horseStr,
		Currency: &currencyStr,
		Amount:   &amountVal,
	}
	_, err := client.OaMyBet(
		context.Background(),
		&g.OaMyBetRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
			Bet:    betN,
		},
	)
	if err != nil {
		log.Fatalf("OaMyBet: %v", err)
	}
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

//export GOaMyActiveBets
func GOaMyActiveBets(clGuid *C.char, activeBets *C.bet_t) C.int {
	clGuidStr := C.GoString(clGuid)
	res, err := client.OaMyActiveBets(
		context.Background(),
		&g.OaMyActiveBetsRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
		},
	)
	if err != nil {
		log.Fatalf("OaMyActiveBets: %v", err)
	}
	var bets []C.bet_t
	return CopyGoSliceToCArray(
		activeBets,
		res.Bets,
		&bets,
		CBetFromGo,
		maxActiveBetsN,
	)
}

//export GOaMyPastBets
func GOaMyPastBets(clGuid *C.char, page *C.char, nextPage *C.char, pastBets *C.fullbet_t) C.int {
	clGuidStr := C.GoString(clGuid)
	pageStr := C.GoString(page)
	res, err := client.OaMyPastBets(
		context.Background(),
		&g.OaMyPastBetsRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
			Page:   &pageStr,
		},
	)
	if err != nil {
		log.Fatalf("OaMyPastBets: %v", err)
	}
	StringToC(*res.NextPage, nextPage)
	var bets []C.fullbet_t
	return CopyGoSliceToCArray(
		pastBets,
		res.Bets,
		&bets,
		CFullbetFromGo,
		betsPerPageN,
	)
}

//export GOaMyBetsSummary
func GOaMyBetsSummary(clGuid *C.char, currencySummaries *C.currencySummary_t) C.int {
	clGuidStr := C.GoString(clGuid)
	res, err := client.OaMyBetsSummary(
		context.Background(),
		&g.OaMyBetsSummaryRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuidStr},
		},
	)
	if err != nil {
		log.Fatalf("OaMyBetsSummary: %v", err)
	}
	var summaries []C.currencySummary_t
	return CopyGoSliceToCArray(
		currencySummaries,
		res.CurrencySummaries,
		&summaries,
		CCurrencySummaryFromGo,
		currenciesN,
	)
}

func main() {}
