#!/bin/bash

mkdir -p generated
protoc \
    --go_out=plugins=grpc:generated \
    api.proto

mkdir -p oa_mod/code/game/generated
protoc-c \
    --c_out=oa_mod/code/game/generated \
    api.proto

sed 's/\<_Oatot_serviceDesc\>/Oatot_serviceDesc/g' \
    -i generated/api.pb.go
