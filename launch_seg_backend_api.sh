#!/bin/sh
./bazel-bin/kcws/cc/seg_backend_api --model_path=./kcws/models/seg_model.pbtxt --vocab_path=./kcws/models/basic_vocab.txt --max_sentence_len=80