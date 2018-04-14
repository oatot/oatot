package lib

import (
	"encoding/json"
	g "github.com/oatot/oatot/generated"
	"golang.org/x/net/context"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"math/big"
	"sync"
)

const (
	// Bet states.
	ACTIVE    = iota
	PAST      = iota
	CANCELLED = iota
)

type MapOfMaps map[string]map[string]uint64
type MapStrToInt map[string]int64

type Player struct {
	FreeMoney     MapStrToInt      `json:"free_money"`
	ActiveBets    map[int]struct{} `json:"active_bets"`
	PastBets      []int            `json:"past_bets"`
	CancelledBets map[int]struct{} `json:"cancelled_bets"`
}

type Bet struct {
	Player   string `json:"player"`
	Horse    string `json:"horse"`
	Currency string `json:"currency"`
	Amount   int64  `json:"amount"`
	Prize    int64  `json:"prize"`
	Winner   string `json:"winner"`
	State    int    `json:"state"`
}

type Data struct {
	Stage      g.GameStage        `json:"stage"`
	Players    map[string]*Player `json:"players"`
	Bets       []*Bet             `json:"bets"`
	ActiveBets map[int]struct{}   `json:"active_bets"`
}

type Server struct {
	m sync.Mutex

	data Data

	StartMoney int64
}

const pastBetsPageSize = 15

var Currencies = []string{
	"OAC",
	"BTC",
}

var Horses = []string{
	"red",
	"blue",
}

func valueInSlice(value string, slice []string) bool {
	for _, val := range slice {
		if value == val {
			return true
		}
	}
	return false
}

func New() (*Server, error) {
	return &Server{
		data: Data{
			Players:    make(map[string]*Player),
			ActiveBets: make(map[int]struct{}),
		},
		StartMoney: 1000,
	}, nil
}

func Load(data []byte) (*Server, error) {
	s := &Server{}
	if err := json.Unmarshal(data, &s.data); err != nil {
		return nil, err
	}
	if s.data.Players == nil {
		s.data.Players = make(map[string]*Player)
	}
	if s.data.ActiveBets == nil {
		s.data.ActiveBets = make(map[int]struct{})
	}
	return s, nil
}

func (s *Server) Save() ([]byte, error) {
	s.m.Lock()
	defer s.m.Unlock()
	return json.Marshal(&s.data)
}

func (s *Server) SetStartMoney(startMoney int64) {
	s.m.Lock()
	defer s.m.Unlock()
	s.StartMoney = startMoney
}

func (s *Server) SiteLoginStep1(ctx context.Context, req *g.SiteLoginStep1Request) (*g.SiteLoginStep1Response, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteLoginStep2(ctx context.Context, req *g.SiteLoginStep2Request) (*g.SiteLoginStep2Response, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteLogout(ctx context.Context, req *g.SiteLogoutRequest) (*g.SiteLogoutResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaLoginStep1(ctx context.Context, req *g.OaLoginStep1Request) (*g.OaLoginStep1Response, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteOaLoginStep2(ctx context.Context, req *g.SiteOaLoginStep2Request) (*g.SiteOaLoginStep2Response, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteMyClGuids(ctx context.Context, req *g.SiteMyClGuidsRequest) (*g.SiteMyClGuidsResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteRemoveClGuid(ctx context.Context, req *g.SiteRemoveClGuidRequest) (*g.SiteRemoveClGuidResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteDepositBtc(ctx context.Context, req *g.SiteDepositBtcRequest) (*g.SiteDepositBtcResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) SiteWithdrawBtc(ctx context.Context, req *g.SiteWithdrawBtcRequest) (*g.SiteWithdrawBtcResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaDiscardBet(ctx context.Context, req *g.OaDiscardBetRequest) (*g.OaDiscardBetResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != g.GameStage_MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	id := int(*req.BetId)
	if id < 0 || id >= len(s.data.Bets) {
		return nil, status.Errorf(codes.NotFound, "No such bet")
	}
	bet := s.data.Bets[id]
	if bet.State != ACTIVE {
		return nil, status.Errorf(codes.FailedPrecondition, "The bet is not active")
	}
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	// Modify.
	bet.State = CANCELLED
	delete(s.data.ActiveBets, id)
	delete(player.ActiveBets, id)
	player.CancelledBets[id] = struct{}{}
	player.FreeMoney[bet.Currency] += bet.Amount
	return &g.OaDiscardBetResponse{}, nil
}

func (s *Server) OaTransferMoney(ctx context.Context, req *g.OaTransferMoneyRequest) (*g.OaTransferMoneyResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != g.GameStage_PLAYING {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	if *req.Currency != "OAC" {
		return nil, status.Errorf(codes.Aborted, "OaTransferMoney is implemented only for OAC")
	}
	player.FreeMoney[*req.Currency] += int64(*req.Amount)
	return &g.OaTransferMoneyResponse{}, nil
}

func initBetSumsAmountsMap() MapOfMaps {
	amountsMap := make(MapOfMaps)
	for _, currency := range Currencies {
		amountsMap[currency] = make(map[string]uint64)
		for _, horse := range Horses {
			amountsMap[currency][horse] = uint64(0)
		}
	}
	return amountsMap
}

func newBetSum(amount uint64, currency, horse string) *g.BetSum {
	return &g.BetSum{Amount: &amount, Currency: &currency, Horse: &horse}
}

func (s *Server) OaActiveBetsSums(ctx context.Context, req *g.OaActiveBetsSumsRequest) (*g.OaActiveBetsSumsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	amountsMap := initBetSumsAmountsMap()
	var betSums []*g.BetSum
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		amountsMap[bet.Currency][bet.Horse] += uint64(bet.Amount)
	}
	for currency, value := range amountsMap {
		for horse, value2 := range value {
			betSums = append(betSums, newBetSum(value2, currency, horse))
		}
	}
	res := &g.OaActiveBetsSumsResponse{BetSums: betSums}
	return res, nil
}

func (s *Server) OaChangeGameStage(ctx context.Context, req *g.OaChangeGameStageRequest) (*g.OaChangeGameStageResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	stage := *req.NewStage
	if stage < g.GameStage_FORMING_TEAMS || stage > g.GameStage_PLAYING {
		return nil, status.Errorf(codes.Aborted, "Bad stage")
	}
	s.data.Stage = stage
	return &g.OaChangeGameStageResponse{}, nil
}

func (s *Server) OaIsNew(ctx context.Context, req *g.OaIsNewRequest) (*g.OaIsNewResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	_, has := s.data.Players[*req.OaAuth.ClGuid]
	result := !has
	return &g.OaIsNewResponse{Result: &result}, nil
}

func basicFreeMoney(startMoney int64) MapStrToInt {
	freeMoney := make(MapStrToInt)
	for _, currency := range Currencies {
		if currency == "OAC" {
			freeMoney[currency] = startMoney
		} else {
			freeMoney[currency] = 0
		}
	}
	return freeMoney
}

func (s *Server) OaRegister(ctx context.Context, req *g.OaRegisterRequest) (*g.OaRegisterResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if _, has := s.data.Players[*req.OaAuth.ClGuid]; has {
		return nil, status.Errorf(codes.AlreadyExists, "AlreadyExists")
	}
	s.data.Players[*req.OaAuth.ClGuid] = &Player{
		FreeMoney:     basicFreeMoney(s.StartMoney),
		ActiveBets:    make(map[int]struct{}),
		CancelledBets: make(map[int]struct{}),
	}
	return &g.OaRegisterResponse{}, nil
}

func newBalance(freeMoney, moneyOnBets uint64, currency string) *g.Balance {
	return &g.Balance{FreeMoney: &freeMoney, MoneyOnBets: &moneyOnBets, Currency: &currency}
}

func (s *Server) OaMyBalance(ctx context.Context, req *g.OaMyBalanceRequest) (*g.OaMyBalanceResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyBalanceResponse{}
	for currency, value := range player.FreeMoney {
		moneyOnBets := uint64(0)
		freeMoney := uint64(value)
		for betID := range player.ActiveBets {
			bet := s.data.Bets[betID]
			if bet.Currency == currency {
				moneyOnBets += uint64(bet.Amount)
			}
		}
		res.Balances = append(res.Balances, newBalance(freeMoney, moneyOnBets, currency))
	}
	return res, nil
}

func (s *Server) OaMyBet(ctx context.Context, req *g.OaMyBetRequest) (*g.OaMyBetResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != g.GameStage_MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	if !valueInSlice(*req.Bet.Currency, Currencies) {
		return nil, status.Errorf(codes.Aborted, "Bad currency")
	}
	if !valueInSlice(*req.Bet.Horse, Horses) {
		return nil, status.Errorf(codes.Aborted, "Bad horse")
	}
	freeMoney := uint64(player.FreeMoney[*req.Bet.Currency])
	if *req.Bet.Amount > freeMoney {
		return nil, status.Errorf(codes.FailedPrecondition, "Not enough money")
	}
	if *req.Bet.Amount <= 0 {
		return nil, status.Errorf(codes.Aborted, "Negative amount")
	}
	betID := len(s.data.Bets)
	bet := &Bet{
		Player:   *req.OaAuth.ClGuid,
		Horse:    *req.Bet.Horse,
		Currency: *req.Bet.Currency,
		Amount:   int64(*req.Bet.Amount),
		State:    ACTIVE,
	}
	s.data.Bets = append(s.data.Bets, bet)
	s.data.ActiveBets[betID] = struct{}{}
	player.ActiveBets[betID] = struct{}{}
	player.FreeMoney[*req.Bet.Currency] -= int64(*req.Bet.Amount)
	return &g.OaMyBetResponse{}, nil
}

func (s *Server) OaCloseBets(ctx context.Context, req *g.OaCloseBetsRequest) (*g.OaCloseBetsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != g.GameStage_PLAYING {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	amountsSums := make(map[string]int64)
	winnersSums := make(map[string]int64)
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		amountsSums[bet.Currency] += bet.Amount
		if bet.Horse == *req.Winner {
			winnersSums[bet.Currency] += bet.Amount
		}
	}
	winnersSums2 := make(map[string]int64)
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		if bet.Horse == *req.Winner {
			x := big.NewInt(amountsSums[bet.Currency])
			x.Mul(x, big.NewInt(bet.Amount))
			x.Div(x, big.NewInt(winnersSums[bet.Currency]))
			bet.Prize = x.Int64()
			winnersSums2[bet.Currency] += bet.Prize
		}
	}
	// Give the remaining small money to somebody.
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		if bet.Horse == *req.Winner {
			if winnersSums2[bet.Currency] < amountsSums[bet.Currency] {
				bet.Prize += amountsSums[bet.Currency] - winnersSums2[bet.Currency]
				winnersSums2[bet.Currency] = amountsSums[bet.Currency]
			}
		}
	}
	var betIDs []int
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		if bet.Prize != 0 && bet.Prize < bet.Amount {
			panic("Something is rotten in the state of Denmark")
		}
		player := s.data.Players[bet.Player]
		player.FreeMoney[bet.Currency] += bet.Prize
		delete(player.ActiveBets, betID)
		player.PastBets = append(player.PastBets, betID)
		bet.Winner = *req.Winner
		betIDs = append(betIDs, betID)
	}
	for _, betID := range betIDs {
		delete(s.data.ActiveBets, betID)
	}
	return &g.OaCloseBetsResponse{}, nil
}

func (s *Server) OaCloseBetsByIncident(ctx context.Context, req *g.OaCloseBetsByIncidentRequest) (*g.OaCloseBetsByIncidentResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	var betIDs []int
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		player := s.data.Players[bet.Player]
		player.FreeMoney[bet.Currency] += bet.Amount
		delete(player.ActiveBets, betID)
		player.CancelledBets[betID] = struct{}{}
		betIDs = append(betIDs, betID)
	}
	for _, betID := range betIDs {
		delete(s.data.ActiveBets, betID)
	}
	return &g.OaCloseBetsByIncidentResponse{}, nil
}

func betToPb(bet *Bet, betID int) *g.Bet {
	id := uint64(betID)
	amount := uint64(bet.Amount)
	prize := uint64(bet.Prize)
	return &g.Bet{
		Horse:    &bet.Horse,
		Currency: &bet.Currency,
		Amount:   &amount,
		Winner:   &bet.Winner,
		Prize:    &prize,
		BetId:    &id,
	}
}

func (s *Server) OaMyActiveBets(ctx context.Context, req *g.OaMyActiveBetsRequest) (*g.OaMyActiveBetsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyActiveBetsResponse{}
	for betID := range player.ActiveBets {
		bet := s.data.Bets[betID]
		res.Bets = append(res.Bets, betToPb(bet, betID))
	}
	return res, nil
}

func (s *Server) OaMyPastBets(ctx context.Context, req *g.OaMyPastBetsRequest) (*g.OaMyPastBetsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	// TODO: Implement next page logic.
	nextPage := ""
	res := &g.OaMyPastBetsResponse{
		NextPage: &nextPage,
	}
	var lastBets []int
	if len(player.PastBets) > pastBetsPageSize {
		lastBets = player.PastBets[len(player.PastBets)-pastBetsPageSize:]
	} else {
		lastBets = player.PastBets
	}
	for betID := range lastBets {
		bet := s.data.Bets[betID]
		res.Bets = append(res.Bets, betToPb(bet, betID))
	}
	return res, nil
}

func defaultCurrencySummary(currency string) *g.CurrencySummary {
	var totalBet, totalPrize, totalLost, betsWon, betsLost uint64
	return &g.CurrencySummary{
		TotalBet:   &totalBet,
		TotalPrize: &totalPrize,
		TotalLost:  &totalLost,
		BetsWon:    &betsWon,
		BetsLost:   &betsLost,
		Currency:   &currency,
	}
}

func (s *Server) OaMyBetsSummary(ctx context.Context, req *g.OaMyBetsSummaryRequest) (*g.OaMyBetsSummaryResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	currencySummaries := make(map[string]*g.CurrencySummary)
	for _, currency := range Currencies {
		currencySummaries[currency] = defaultCurrencySummary(currency)
	}
	for betID := range player.PastBets {
		bet := s.data.Bets[betID]
		summary := currencySummaries[bet.Currency]
		*summary.TotalBet += uint64(bet.Amount)
		*summary.TotalPrize += uint64(bet.Prize)
		if bet.Prize == 0 {
			*summary.TotalLost += uint64(bet.Amount)
			*summary.BetsLost += 1
		} else {
			*summary.BetsWon += 1
		}
	}
	res := &g.OaMyBetsSummaryResponse{}
	for _, value := range currencySummaries {
		res.CurrencySummaries = append(res.CurrencySummaries, value)
	}
	return res, nil
}
