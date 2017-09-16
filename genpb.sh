#!/bin/bash

mkdir -p backend/generated
protoc \
    --go_out=plugins=grpc:backend/generated \
    api.proto

mkdir -p oa_mod/code/game/generated
protoc-c \
    --c_out=oa_mod/code/game/generated \
    api.proto

sed 's/\<_Oatot_serviceDesc\>/Oatot_serviceDesc/g' \
    -i backend/generated/api.pb.go
