package protobufcrpc

import (
	"encoding/binary"
	"errors"
	"log"
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
	methods      []Method
	methodsNames []string
}

func New(desc *grpc.ServiceDesc, service interface{}) (*Server, error) {
	s := &Server{}
	for _, method := range desc.Methods {
		m := reflect.ValueOf(service).MethodByName(method.MethodName)
		reqType := m.Type().In(1).Elem()
		handler := func(input []byte) ([]byte, error) {
			req := reflect.New(reqType)
			pb := req.Interface().(proto.Message)
			if err := proto.Unmarshal(input, pb); err != nil {
				return nil, err
			}
			ctx := reflect.ValueOf(context.Background())
			results := m.Call([]reflect.Value{ctx, req})
			if results[1].Interface() != nil {
				if err := results[1].Interface().(error); err != nil {
					return nil, err
				}
			}
			pb = results[0].Interface().(proto.Message)
			return proto.Marshal(pb)
		}
		s.methods = append(s.methods, handler)
		s.methodsNames = append(s.methodsNames, method.MethodName)
	}
	return s, nil
}

func (s *Server) Serve(conn net.Conn) error {
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
	status := int32(0)
	if err != nil {
		name := s.methodsNames[methodIndex]
		log.Printf("Method %q returned error: %v.", name, err)
		status = 1
		output = nil
	}
	// Write.
	if err := binary.Write(conn, binary.LittleEndian, status); err != nil {
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
