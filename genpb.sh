#!/bin/bash

mkdir -p backend/generated
protoc \
    --go_out=plugins=grpc:backend/generated \
    api.proto

mkdir -p oa_mod/code/game/generated
protoc-c \
    --c_out=oa_mod/code/game/generated \
    api.proto
