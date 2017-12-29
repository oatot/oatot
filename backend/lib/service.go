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
	// Game stages. TODO: put this enum to api.proto.
	FORMING_TEAMS = iota
	MAKING_BETS   = iota
	PLAYING       = iota
)

const (
	// Bid states.
	ACTIVE    = iota
	PAST      = iota
	CANCELLED = iota
)

type Player struct {
	FreeMoney     map[string]int64 `json:"free_money"`
	ActiveBids    map[int]struct{} `json:"active_bids"`
	PastBids      map[int]struct{} `json:"past_bids"`
	CancelledBids map[int]struct{} `json:"cancelled_bids"`
}

type Bid struct {
	Player   string `json:"player"`
	Horse    string `json:"horse"`
	Currency string `json:"currency"`
	Amount   int64  `json:"amount"`
	Prize    int64  `json:"prize"`
	Winner   string `json:"winner"`
	State    int    `json:"state"`
}

type Data struct {
	Stage      int                `json:"stage"`
	Players    map[string]*Player `json:"players"`
	Bids       []*Bid             `json:"bids"`
	ActiveBids map[int]struct{}   `json:"active_bids"`
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
			ActiveBids: make(map[int]struct{}),
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
	if s.data.ActiveBids == nil {
		s.data.ActiveBids = make(map[int]struct{})
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
	if s.data.Stage != MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	id := int(*req.BetId)
	if id < 0 || id >= len(s.data.Bids) {
		return nil, status.Errorf(codes.NotFound, "No such bid")
	}
	bid := s.data.Bids[id]
	if bid.State != ACTIVE {
		return nil, status.Errorf(codes.FailedPrecondition, "The bid is not active")
	}
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	// Modify.
	bid.State = CANCELLED
	delete(s.data.ActiveBids, id)
	delete(player.ActiveBids, id)
	player.CancelledBids[id] = struct{}{}
	player.FreeMoney[bid.Currency] += bid.Amount
	return &g.OaDiscardBetResponse{}, nil
}

func (s *Server) OaTransferMoney(ctx context.Context, req *g.OaTransferMoneyRequest) (*g.OaTransferMoneyResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != PLAYING {
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

func (s *Server) OaActiveBidsSums(ctx context.Context, req *g.OaActiveBidsSumsRequest) (*g.OaActiveBidsSumsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	var oacAmount, btcAmount uint64
	for bidID := range s.data.ActiveBids {
		bid := s.data.Bids[bidID]
		if bid.Horse != *req.Horse {
			continue
		}
		if bid.Currency == "OAC" {
			oacAmount += uint64(bid.Amount)
		} else if bid.Currency == "BTC" {
			btcAmount += uint64(bid.Amount)
		}
	}
	return &g.OaActiveBidsSumsResponse{
		OacAmount: &oacAmount,
		BtcAmount: &btcAmount,
	}, nil
}

func (s *Server) OaChangeGameStage(ctx context.Context, req *g.OaChangeGameStageRequest) (*g.OaChangeGameStageResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	stage := int(*req.NewStage)
	if stage < FORMING_TEAMS || stage > PLAYING {
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
		ActiveBids:    make(map[int]struct{}),
		PastBids:      make(map[int]struct{}),
		CancelledBids: make(map[int]struct{}),
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
	moneyOnBids := uint64(0)
	for bidID := range player.ActiveBids {
		bid := s.data.Bids[bidID]
		if bid.Currency == *req.Currency {
			moneyOnBids += uint64(bid.Amount)
		}
	}
	return &g.OaMyBalanceResponse{
		FreeMoney:   &freeMoney,
		MoneyOnBids: &moneyOnBids,
	}, nil
}

func (s *Server) OaMyBid(ctx context.Context, req *g.OaMyBidRequest) (*g.OaMyBidResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	if *req.Bid.Currency != "OAC" && *req.Bid.Currency != "BTC" {
		return nil, status.Errorf(codes.Aborted, "Bad currency")
	}
	if *req.Bid.Horse != "red" && *req.Bid.Horse != "blue" {
		return nil, status.Errorf(codes.Aborted, "Bad horse")
	}
	freeMoney := uint64(player.FreeMoney[*req.Bid.Currency])
	if *req.Bid.Amount > freeMoney {
		return nil, status.Errorf(codes.FailedPrecondition, "Not enough money")
	}
	if *req.Bid.Amount <= 0 {
		return nil, status.Errorf(codes.Aborted, "Negative amount")
	}
	bidID := len(s.data.Bids)
	bid := &Bid{
		Player:   *req.OaAuth.ClGuid,
		Horse:    *req.Bid.Horse,
		Currency: *req.Bid.Currency,
		Amount:   int64(*req.Bid.Amount),
		State:    ACTIVE,
	}
	s.data.Bids = append(s.data.Bids, bid)
	s.data.ActiveBids[bidID] = struct{}{}
	player.ActiveBids[bidID] = struct{}{}
	player.FreeMoney[*req.Bid.Currency] -= int64(*req.Bid.Amount)
	return &g.OaMyBidResponse{}, nil
}

func (s *Server) OaCloseBids(ctx context.Context, req *g.OaCloseBidsRequest) (*g.OaCloseBidsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != PLAYING {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	amountsSums := make(map[string]int64)
	winnersSums := make(map[string]int64)
	for bidID := range s.data.ActiveBids {
		bid := s.data.Bids[bidID]
		amountsSums[bid.Currency] += bid.Amount
		if bid.Horse == *req.Winner {
			winnersSums[bid.Currency] += bid.Amount
		}
	}
	winnersSums2 := make(map[string]int64)
	for bidID := range s.data.ActiveBids {
		bid := s.data.Bids[bidID]
		if bid.Horse == *req.Winner {
			x := big.NewInt(amountsSums[bid.Currency])
			x.Mul(x, big.NewInt(bid.Amount))
			x.Div(x, big.NewInt(winnersSums[bid.Currency]))
			bid.Prize = x.Int64()
			winnersSums2[bid.Currency] += bid.Prize
		}
	}
	// Give the remaining small money to somebody.
	for bidID := range s.data.ActiveBids {
		bid := s.data.Bids[bidID]
		if bid.Horse == *req.Winner {
			if winnersSums2[bid.Currency] < amountsSums[bid.Currency] {
				bid.Prize += amountsSums[bid.Currency] - winnersSums2[bid.Currency]
				winnersSums2[bid.Currency] = amountsSums[bid.Currency]
			}
		}
	}
	var bidIDs []int
	for bidID := range s.data.ActiveBids {
		bid := s.data.Bids[bidID]
		if bid.Prize != 0 && bid.Prize < bid.Amount {
			panic("Something is rotten in the state of Denmark")
		}
		player := s.data.Players[bid.Player]
		player.FreeMoney[bid.Currency] += bid.Prize
		delete(player.ActiveBids, bidID)
		player.PastBids[bidID] = struct{}{}
		bid.Winner = *req.Winner
		bidIDs = append(bidIDs, bidID)
	}
	for _, bidID := range bidIDs {
		delete(s.data.ActiveBids, bidID)
	}
	return &g.OaCloseBidsResponse{}, nil
}

func (s *Server) OaCloseBidsByIncident(ctx context.Context, req *g.OaCloseBidsByIncidentRequest) (*g.OaCloseBidsByIncidentResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.data.Stage != PLAYING && s.data.Stage != MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	var bidIDs []int
	for bidID := range s.data.ActiveBids {
		bid := s.data.Bids[bidID]
		player := s.data.Players[bid.Player]
		player.FreeMoney[bid.Currency] += bid.Amount
		delete(player.ActiveBids, bidID)
		player.CancelledBids[bidID] = struct{}{}
		bidIDs = append(bidIDs, bidID)
	}
	for _, bidID := range bidIDs {
		delete(s.data.ActiveBids, bidID)
	}
	return &g.OaCloseBidsByIncidentResponse{}, nil
}

func bidToPb(bid *Bid, bidID int) *g.Bid {
	id := uint64(bidID)
	amount := uint64(bid.Amount)
	prize := uint64(bid.Prize)
	return &g.Bid{
		Horse:    &bid.Horse,
		Currency: &bid.Currency,
		Amount:   &amount,
		Winner:   &bid.Winner,
		Prize:    &prize,
		BetId:    &id,
	}
}

func (s *Server) OaMyActiveBids(ctx context.Context, req *g.OaMyActiveBidsRequest) (*g.OaMyActiveBidsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyActiveBidsResponse{}
	for bidID := range player.ActiveBids {
		bid := s.data.Bids[bidID]
		res.Bids = append(res.Bids, bidToPb(bid, bidID))
	}
	return res, nil
}

func (s *Server) OaMyPastBids(ctx context.Context, req *g.OaMyPastBidsRequest) (*g.OaMyPastBidsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	// TODO: Implement next page logic.
	nextPage := ""
	res := &g.OaMyPastBidsResponse{
		NextPage: &nextPage,
	}
	for bidID := range player.PastBids {
		bid := s.data.Bids[bidID]
		res.Bids = append(res.Bids, bidToPb(bid, bidID))
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

func (s *Server) OaMyBidsSummary(ctx context.Context, req *g.OaMyBidsSummaryRequest) (*g.OaMyBidsSummaryResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.data.Players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyBidsSummaryResponse{
		OacSummary: defaultCurrencySummary(),
		BtcSummary: defaultCurrencySummary(),
	}
	for bidID := range player.PastBids {
		bid := s.data.Bids[bidID]
		var summary *g.CurrencySummary
		if bid.Currency == "OAC" {
			summary = res.OacSummary
		} else if bid.Currency == "BTC" {
			summary = res.BtcSummary
		} else {
			continue
		}
		*summary.TotalBet += uint64(bid.Amount)
		*summary.TotalPrize += uint64(bid.Prize)
		if bid.Prize == 0 {
			*summary.TotalLost += uint64(bid.Amount)
			*summary.BetsLost += 1
		} else {
			*summary.BetsWon += 1
		}
	}
	return res, nil
}
