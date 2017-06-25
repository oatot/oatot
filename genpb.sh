#!/bin/bash

mkdir -p backend/generated
protoc \
    --go_out=plugins=grpc:backend/generated \
    api.proto

mkdir -p mod_proxy/generated
protoc \
    --go_out=plugins=grpc:mod_proxy/generated \
    api.proto
