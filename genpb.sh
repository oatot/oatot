#!/bin/bash

mkdir -p backend/generated
protoc \
    --go_out=plugins=grpc:backend/generated \
    api.proto

mkdir -p oa/generated
protoc-c \
    --c_out=oa/generated \
    api.proto
