#! /bin/bash

if [ -e "hello.txt" ]; then
	rm hello.txt
fi

echo "Hello from process id=$$" >hello.txt
echo "Args: $@" >>hello.txt

sleep 2
