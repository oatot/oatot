package lib

import (
	"encoding/json"
	"math/big"
	"sync"

	g "github.com/oatot/oatot/generated"
	"golang.org/x/net/context"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const (
	// Bet states.
	ACTIVE    = iota
	PAST      = iota
	CANCELLED = iota
)

type Player struct {
	FreeMoney     map[string]int64 `json:"free_money"`
	ActiveBets    map[int]struct{} `json:"active_bets"`
	PastBets      map[int]struct{} `json:"past_bets"`
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

	startMoney int64
}

func New() (*Server, error) {
	return &Server{
		data: Data{
			Players:    make(map[string]*Player),
			ActiveBets: make(map[int]struct{}),
		},
		startMoney: 1000,
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
	s.startMoney = startMoney
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

func (s *Server) OaActiveBetsSums(ctx context.Context, req *g.OaActiveBetsSumsRequest) (*g.OaActiveBetsSumsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	var oacAmount, btcAmount uint64
	for betID := range s.data.ActiveBets {
		bet := s.data.Bets[betID]
		if bet.Horse != *req.Horse {
			continue
		}
		if bet.Currency == "OAC" {
			oacAmount += uint64(bet.Amount)
		} else if bet.Currency == "BTC" {
			btcAmount += uint64(bet.Amount)
		}
	}
	return &g.OaActiveBetsSumsResponse{
		OacAmount: &oacAmount,
		BtcAmount: &btcAmount,
	}, nil
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

func (s *Server) OaRegister(ctx context.Context, req *g.OaRegisterRequest) (*g.OaRegisterResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if _, has := s.data.Players[*req.OaAuth.ClGuid]; has {
		return nil, status.Errorf(codes.AlreadyExists, "AlreadyExists")
	}
	s.data.Players[*req.OaAuth.ClGuid] = &Player{
		FreeMoney:     map[string]int64{"OAC": s.startMoney},
		ActiveBets:    make(map[int]struct{}),
		PastBets:      make(map[int]struct{}),
		CancelledBets: make(map[int]struct{}),
	}
	return &g.OaRegisterResponse{}, nil
}

func (s *Server) OaMyBalance(ctx context.Context, req *g.OaMyBalanceRequest) (*g.OaMyBalanceResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	freeMoney := uint64(player.FreeMoney[*req.Currency])
	moneyOnBets := uint64(0)
	for betID := range player.ActiveBets {
		bet := s.data.Bets[betID]
		if bet.Currency == *req.Currency {
			moneyOnBets += uint64(bet.Amount)
		}
	}
	return &g.OaMyBalanceResponse{
		FreeMoney:   &freeMoney,
		MoneyOnBets: &moneyOnBets,
	}, nil
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
	if *req.Bet.Currency != "OAC" && *req.Bet.Currency != "BTC" {
		return nil, status.Errorf(codes.Aborted, "Bad currency")
	}
	if *req.Bet.Horse != "red" && *req.Bet.Horse != "blue" {
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
		player.PastBets[betID] = struct{}{}
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
	for betID := range player.PastBets {
		bet := s.data.Bets[betID]
		res.Bets = append(res.Bets, betToPb(bet, betID))
	}
	return res, nil
}

func defaultCurrencySummary() *g.CurrencySummary {
	var totalBet, totalPrize, totalLost, betsWon, betsLost uint64
	return &g.CurrencySummary{
		TotalBet:   &totalBet,
		TotalPrize: &totalPrize,
		TotalLost:  &totalLost,
		BetsWon:    &betsWon,
		BetsLost:   &betsLost,
	}
}

func (s *Server) OaMyBetsSummary(ctx context.Context, req *g.OaMyBetsSummaryRequest) (*g.OaMyBetsSummaryResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyBetsSummaryResponse{
		OacSummary: defaultCurrencySummary(),
		BtcSummary: defaultCurrencySummary(),
	}
	for betID := range player.PastBets {
		bet := s.data.Bets[betID]
		var summary *g.CurrencySummary
		if bet.Currency == "OAC" {
			summary = res.OacSummary
		} else if bet.Currency == "BTC" {
			summary = res.BtcSummary
		} else {
			continue
		}
		*summary.TotalBet += uint64(bet.Amount)
		*summary.TotalPrize += uint64(bet.Prize)
		if bet.Prize == 0 {
			*summary.TotalLost += uint64(bet.Amount)
			*summary.BetsLost += 1
		} else {
			*summary.BetsWon += 1
		}
	}
	return res, nil
}
