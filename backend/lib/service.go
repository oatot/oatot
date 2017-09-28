package lib

import (
	"math/big"
	"sync"

	g "github.com/oatot/oatot/backend/generated"
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
	freeMoney     map[string]int64
	activeBids    map[int]struct{}
	pastBids      map[int]struct{}
	cancelledBids map[int]struct{}
}

type Bid struct {
	player        string
	horse         string
	currency      string
	amount, prize int64
	winner        string
	state         int
}

type Server struct {
	m sync.Mutex

	stage int

	players map[string]*Player
	bids    []*Bid

	activeBids map[int]struct{}
}

func New() (*Server, error) {
	return &Server{
		players:    make(map[string]*Player),
		activeBids: make(map[int]struct{}),
	}, nil
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
	if s.stage != MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	id := int(*req.BetId)
	if id < 0 || id >= len(s.bids) {
		return nil, status.Errorf(codes.NotFound, "No such bid")
	}
	bid := s.bids[id]
	if bid.state != ACTIVE {
		return nil, status.Errorf(codes.FailedPrecondition, "The bid is not active")
	}
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	// Modify.
	bid.state = CANCELLED
	delete(s.activeBids, id)
	delete(player.activeBids, id)
	player.cancelledBids[id] = struct{}{}
	player.freeMoney[bid.currency] += bid.amount
	return &g.OaDiscardBetResponse{}, nil
}

func (s *Server) OaTransferMoney(ctx context.Context, req *g.OaTransferMoneyRequest) (*g.OaTransferMoneyResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.stage != PLAYING {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	if *req.Currency != "OAC" {
		return nil, status.Errorf(codes.Aborted, "OaTransferMoney is implemented only for OAC")
	}
	player.freeMoney[*req.Currency] += int64(*req.Amount)
	return &g.OaTransferMoneyResponse{}, nil
}

func (s *Server) OaActiveBidsSums(ctx context.Context, req *g.OaActiveBidsSumsRequest) (*g.OaActiveBidsSumsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	var oacAmount, btcAmount uint64
	for bidID := range s.activeBids {
		bid := s.bids[bidID]
		if bid.horse != *req.Horse {
			continue
		}
		if bid.currency == "OAC" {
			oacAmount += uint64(bid.amount)
		} else if bid.currency == "BTC" {
			btcAmount += uint64(bid.amount)
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
	s.stage = stage
	return &g.OaChangeGameStageResponse{}, nil
}

func (s *Server) OaIsNew(ctx context.Context, req *g.OaIsNewRequest) (*g.OaIsNewResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	_, has := s.players[*req.OaAuth.ClGuid]
	result := !has
	return &g.OaIsNewResponse{Result: &result}, nil
}

func (s *Server) OaRegister(ctx context.Context, req *g.OaRegisterRequest) (*g.OaRegisterResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	const startMoney = 1000
	if _, has := s.players[*req.OaAuth.ClGuid]; has {
		return nil, status.Errorf(codes.AlreadyExists, "AlreadyExists")
	}
	s.players[*req.OaAuth.ClGuid] = &Player{
		freeMoney:     map[string]int64{"OAC": startMoney},
		activeBids:    make(map[int]struct{}),
		pastBids:      make(map[int]struct{}),
		cancelledBids: make(map[int]struct{}),
	}
	return &g.OaRegisterResponse{}, nil
}

func (s *Server) OaMyBalance(ctx context.Context, req *g.OaMyBalanceRequest) (*g.OaMyBalanceResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	freeMoney := uint64(player.freeMoney[*req.Currency])
	moneyOnBids := uint64(0)
	for bidID := range player.activeBids {
		bid := s.bids[bidID]
		if bid.currency == *req.Currency {
			moneyOnBids += uint64(bid.amount)
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
	if s.stage != MAKING_BETS {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	if *req.Bid.Currency != "OAC" && *req.Bid.Currency != "BTC" {
		return nil, status.Errorf(codes.Aborted, "Bad currency")
	}
	if *req.Bid.Horse != "red" && *req.Bid.Horse != "blue" {
		return nil, status.Errorf(codes.Aborted, "Bad horse")
	}
	freeMoney := uint64(player.freeMoney[*req.Bid.Currency])
	if *req.Bid.Amount > freeMoney {
		return nil, status.Errorf(codes.FailedPrecondition, "Not enough money")
	}
	if *req.Bid.Amount <= 0 {
		return nil, status.Errorf(codes.Aborted, "Negative amount")
	}
	bidID := len(s.bids)
	bid := &Bid{
		player:   *req.OaAuth.ClGuid,
		horse:    *req.Bid.Horse,
		currency: *req.Bid.Currency,
		amount:   int64(*req.Bid.Amount),
		state:    ACTIVE,
	}
	s.bids = append(s.bids, bid)
	s.activeBids[bidID] = struct{}{}
	player.activeBids[bidID] = struct{}{}
	player.freeMoney[*req.Bid.Currency] -= int64(*req.Bid.Amount)
	return &g.OaMyBidResponse{}, nil
}

func (s *Server) OaCloseBids(ctx context.Context, req *g.OaCloseBidsRequest) (*g.OaCloseBidsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.stage != PLAYING {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	amountsSums := make(map[string]int64)
	winnersSums := make(map[string]int64)
	for bidID := range s.activeBids {
		bid := s.bids[bidID]
		amountsSums[bid.currency] += bid.amount
		if bid.horse == *req.Winner {
			winnersSums[bid.currency] += bid.amount
		}
	}
	winnersSums2 := make(map[string]int64)
	for bidID := range s.activeBids {
		bid := s.bids[bidID]
		if bid.horse == *req.Winner {
			x := big.NewInt(amountsSums[bid.currency])
			x.Mul(x, big.NewInt(bid.amount))
			x.Div(x, big.NewInt(winnersSums[bid.currency]))
			bid.prize = x.Int64()
			winnersSums2[bid.currency] += bid.prize
		}
	}
	// Give the remaining small money to somebody.
	for bidID := range s.activeBids {
		bid := s.bids[bidID]
		if bid.horse == *req.Winner {
			if winnersSums2[bid.currency] < amountsSums[bid.currency] {
				bid.prize += amountsSums[bid.currency] - winnersSums2[bid.currency]
				winnersSums2[bid.currency] = amountsSums[bid.currency]
			}
		}
	}
	var bidIDs []int
	for bidID := range s.activeBids {
		bid := s.bids[bidID]
		if bid.prize != 0 && bid.prize < bid.amount {
			panic("Something is rotten in the state of Denmark")
		}
		player := s.players[bid.player]
		player.freeMoney[bid.currency] += bid.prize
		delete(player.activeBids, bidID)
		player.pastBids[bidID] = struct{}{}
		bid.winner = *req.Winner
		bidIDs = append(bidIDs, bidID)
	}
	for _, bidID := range bidIDs {
		delete(s.activeBids, bidID)
	}
	return &g.OaCloseBidsResponse{}, nil
}

func (s *Server) OaCloseBidsByIncident(ctx context.Context, req *g.OaCloseBidsByIncidentRequest) (*g.OaCloseBidsByIncidentResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	if s.stage != PLAYING {
		return nil, status.Errorf(codes.FailedPrecondition, "Bad stage")
	}
	var bidIDs []int
	for bidID := range s.activeBids {
		bid := s.bids[bidID]
		player := s.players[bid.player]
		player.freeMoney[bid.currency] += bid.amount
		delete(player.activeBids, bidID)
		player.cancelledBids[bidID] = struct{}{}
		bidIDs = append(bidIDs, bidID)
	}
	for _, bidID := range bidIDs {
		delete(s.activeBids, bidID)
	}
	return &g.OaCloseBidsByIncidentResponse{}, nil
}

func bidToPb(bid *Bid, bidID int) *g.Bid {
	id := uint64(bidID)
	amount := uint64(bid.amount)
	prize := uint64(bid.prize)
	return &g.Bid{
		Horse:    &bid.horse,
		Currency: &bid.currency,
		Amount:   &amount,
		Winner:   &bid.winner,
		Prize:    &prize,
		BetId:    &id,
	}
}

func (s *Server) OaMyActiveBids(ctx context.Context, req *g.OaMyActiveBidsRequest) (*g.OaMyActiveBidsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyActiveBidsResponse{}
	for bidID := range player.activeBids {
		bid := s.bids[bidID]
		res.Bids = append(res.Bids, bidToPb(bid, bidID))
	}
	return res, nil
}

func (s *Server) OaMyPastBids(ctx context.Context, req *g.OaMyPastBidsRequest) (*g.OaMyPastBidsResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	// TODO: Implement next page logic.
	nextPage := ""
	res := &g.OaMyPastBidsResponse{
		NextPage: &nextPage,
	}
	for bidID := range player.activeBids {
		bid := s.bids[bidID]
		res.Bids = append(res.Bids, bidToPb(bid, bidID))
	}
	return res, nil
}

func defaultCurrencySummary() *g.CurrencySummary {
	var totalBet, totalPrize, totalLost, betsWon, betsLost uint64
	return &g.CurrencySummary{
		TotalBet: &totalBet,
		TotalPrize: &totalPrize,
		TotalLost: &totalLost,
		BetsWon: &betsWon,
		BetsLost: &betsLost,
	}
}

func (s *Server) OaMyBidsSummary(ctx context.Context, req *g.OaMyBidsSummaryRequest) (*g.OaMyBidsSummaryResponse, error) {
	s.m.Lock()
	defer s.m.Unlock()
	player, has := s.players[*req.OaAuth.ClGuid]
	if !has {
		return nil, status.Errorf(codes.NotFound, "No such player")
	}
	res := &g.OaMyBidsSummaryResponse{
		OacSummary: defaultCurrencySummary(),
		BtcSummary: defaultCurrencySummary(),
	}
	for bidID := range player.pastBids {
		bid := s.bids[bidID]
		var s *g.CurrencySummary
		if bid.currency == "OAC" {
			s = res.OacSummary
		} else if bid.currency == "BTC" {
			s = res.BtcSummary
		} else {
			continue
		}
		*s.TotalBet += uint64(bid.amount)
		*s.TotalPrize += uint64(bid.prize)
		if bid.prize == 0 {
			*s.TotalLost += uint64(bid.amount)
			*s.BetsLost += 1
		} else {
			*s.BetsWon += 1
		}
	}
	return res, nil
}
