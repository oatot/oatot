package lib

import (
	g "github.com/oatot/oatot/backend/generated"
	"golang.org/x/net/context"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type Server struct {
}

func New() (*Server, error) {
	return &Server{}, nil
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
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaTransferMoney(ctx context.Context, req *g.OaTransferMoneyRequest) (*g.OaTransferMoneyResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaActiveBidsSums(ctx context.Context, req *g.OaActiveBidsSumsRequest) (*g.OaActiveBidsSumsResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaChangeGameStage(ctx context.Context, req *g.OaChangeGameStageRequest) (*g.OaChangeGameStageResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaIsNew(ctx context.Context, req *g.OaIsNewRequest) (*g.OaIsNewResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaRegister(ctx context.Context, req *g.OaRegisterRequest) (*g.OaRegisterResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaMyBalance(ctx context.Context, req *g.OaMyBalanceRequest) (*g.OaMyBalanceResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaMyBid(ctx context.Context, req *g.OaMyBidRequest) (*g.OaMyBidResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaCloseBids(ctx context.Context, req *g.OaCloseBidsRequest) (*g.OaCloseBidsResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaCloseBidsByIncident(ctx context.Context, req *g.OaCloseBidsByIncidentRequest) (*g.OaCloseBidsByIncidentResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaMyActiveBids(ctx context.Context, req *g.OaMyActiveBidsRequest) (*g.OaMyActiveBidsResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaMyPastBids(ctx context.Context, req *g.OaMyPastBidsRequest) (*g.OaMyPastBidsResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}

func (s *Server) OaMyBidsSummary(ctx context.Context, req *g.OaMyBidsSummaryRequest) (*g.OaMyBidsSummaryResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Not implemented")
}
