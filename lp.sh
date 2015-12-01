#!/bin/bash
[ -z $LIBPROTOBUF ] && echo `pkg-config --variable=prefix protobuf` || echo $LIBPROTOBUF
