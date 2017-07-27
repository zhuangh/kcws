#!/bin/sh
curl -H "Content-Type: application/json" -X POST -d '{"sentence":"你好吗，我是天才"}' http://127.0.0.1:9090/tf_seg/api