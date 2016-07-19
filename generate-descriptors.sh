#!/bin/bash

for proto in `find . -iname '*.proto' -not -path './node_modules/*'`; do
  proto=${proto:2}
  protoc --proto_path=./ --include_imports=true --descriptor_set_out=${proto%.*}.desc ${proto}
done

