package protobufcrpc

import (
	"encoding/binary"
	"errors"
	"net"
	"reflect"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

var (
	errTooLong   = errors.New("Too long input")
	errBadN      = errors.New("Read or Write returned unexpected n")
	errBadMethod = errors.New("Bad method")
)

type Method func(input []byte) ([]byte, error)

type Server struct {
	methods []Method
}

func New(desc *grpc.ServiceDesc, service interface{}) (*Server, error) {
	s := &Server{}
	for _, method := range desc.Methods {
		ser := reflect.ValueOf(service)
		m := ser.MethodByName(method.MethodName)
		reqType := m.Type().In(2).Elem()
		handler := func(input []byte) ([]byte, error) {
			req := reflect.New(reqType)
			pb := req.Interface().(proto.Message)
			if err := proto.Unmarshal(input, pb); err != nil {
				return nil, err
			}
			ctx := reflect.ValueOf(context.Background())
			results := m.Call([]reflect.Value{ser, ctx, req})
			if err := results[1].Interface().(error); err != nil {
				return nil, err
			}
			pb = results[0].Interface().(proto.Message)
			return proto.Marshal(pb)
		}
		s.methods = append(s.methods, handler)
	}
	return s, nil
}

func (s *Server) ServeRPC(conn net.Conn, server interface{}) error {
	// Read.
	var methodIndex, messageLength, requestID int32
	if err := binary.Read(conn, binary.LittleEndian, &methodIndex); err != nil {
		return err
	}
	if err := binary.Read(conn, binary.LittleEndian, &messageLength); err != nil {
		return err
	}
	if err := binary.Read(conn, binary.LittleEndian, &requestID); err != nil {
		return err
	}
	if messageLength > 4096 {
		return errTooLong
	}
	input := make([]byte, messageLength)
	n, err := conn.Read(input)
	if err != nil {
		return err
	}
	if n != int(messageLength) {
		return errBadN
	}
	if methodIndex < 0 || int(methodIndex) > len(s.methods) {
		return errBadMethod
	}
	// Run.
	output, err := s.methods[methodIndex](input)
	if err != nil {
		return err
	}
	// Write.
	success := int32(0)
	if err := binary.Write(conn, binary.LittleEndian, success); err != nil {
		return err
	}
	if err := binary.Write(conn, binary.LittleEndian, methodIndex); err != nil {
		return err
	}
	if err := binary.Write(conn, binary.LittleEndian, int32(len(output))); err != nil {
		return err
	}
	if err := binary.Write(conn, binary.LittleEndian, requestID); err != nil {
		return err
	}
	n, err = conn.Write(output)
	if err != nil {
		return err
	}
	if n != len(output) {
		return errBadN
	}
	return nil
}
