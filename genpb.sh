#!/bin/bash

protoc \
    --go_out=plugins=grpc:backend/generated \
    --cpp_out=oa/generated \
    --grpc_out=oa/generated \
    --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" \
    api.proto
