package lib

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	g "github.com/oatot/oatot/generated"
	"github.com/phayes/freeport"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"log"
	"net"
	"testing"
)

var (
	currencies = []string{
		"OAC",
		"BTC",
	}
	horses = []string{
		"red",
		"blue",
	}
	testStartMoney   int64  = 10
	defaultBetAmount uint64 = 5
)

func makeClient(startMoney int64) (g.OatotClient, func(), error) {
	server, err := New()
	if err != nil {
		log.Fatalf("lib.New: %v.", err)
	}
	server.SetStartMoney(startMoney)
	grpcServer := grpc.NewServer()
	g.RegisterOatotServer(grpcServer, server)
	grpcAddr := fmt.Sprintf("127.0.0.1:%d", freeport.GetPort())
	lis, err := net.Listen("tcp", grpcAddr)
	go grpcServer.Serve(lis)
	conn, err := grpc.Dial(grpcAddr, grpc.WithInsecure())
	if err != nil {
		lis.Close()
		log.Fatalf("Did not connect: %v.", err)
	}
	return g.NewOatotClient(conn), func() {
		lis.Close()
	}, nil
}

func TestOaDiscardBet(t *testing.T) {
	clGuid0 := "posdkapkakapckaa"
	client, closer, err := makeClient(testStartMoney)
	if err != nil {
		t.Fatalf("makeClient: %v.", err)
	}
	defer closer()
	_, err = client.OaRegister(
		context.Background(),
		&g.OaRegisterRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuid0},
		},
	)
	if err != nil {
		t.Fatalf("OaRegister: %v.", err)
	}
	stage := g.GameStage_MAKING_BETS
	_, err = client.OaChangeGameStage(
		context.Background(),
		&g.OaChangeGameStageRequest{
			NewStage: &stage,
		},
	)
	if err != nil {
		t.Fatalf("OaChangeGameStage: %v.", err)
	}
	_, err = client.OaMyBet(
		context.Background(),
		&g.OaMyBetRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuid0},
			Bet: &g.Bet{
				Horse:    &horses[0],
				Currency: &currencies[0],
				Amount:   &defaultBetAmount,
			},
		},
	)
	if err != nil {
		t.Fatalf("OaMyBet: %v.", err)
	}
	activeBets, err := client.OaMyActiveBets(
		context.Background(),
		&g.OaMyActiveBetsRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuid0},
		},
	)
	if err != nil {
		t.Fatalf("OaMyActiveBets: %v.", err)
	}
	betId := activeBets.Bets[0].BetId
	_, err = client.OaDiscardBet(
		context.Background(),
		&g.OaDiscardBetRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuid0},
			BetId:  betId,
		},
	)
	if err != nil {
		t.Fatalf("OaDiscardBet: %v.", err)
	}
	activeBets, err = client.OaMyActiveBets(
		context.Background(),
		&g.OaMyActiveBetsRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuid0},
		},
	)
	if err != nil {
		t.Fatalf("OaMyActiveBets: %v.", err)
	}
	if len(activeBets.Bets) != 0 {
		t.Errorf("OaDiscardBet didn't discard the bet.")
	}
	res, err := client.OaMyBalance(
		context.Background(),
		&g.OaMyBalanceRequest{
			OaAuth: &g.OaAuth{ClGuid: &clGuid0},
		},
	)
	if err != nil {
		t.Fatalf("OaMyBalance: %v.", err)
	}
	noMoney, freeMoney := uint64(0), uint64(testStartMoney)
	wantedBalances := []*g.Balance{
		&g.Balance{FreeMoney: &freeMoney, MoneyOnBets: &noMoney, Currency: &currencies[0]},
		&g.Balance{FreeMoney: &noMoney, MoneyOnBets: &noMoney, Currency: &currencies[1]},
	}
	for i := range wantedBalances {
		if !proto.Equal(wantedBalances[i], res.Balances[i]) {
			t.Errorf("OaDiscardBet didn't return the money back.")
		}
	}
}

// TODO: finish the tests.
