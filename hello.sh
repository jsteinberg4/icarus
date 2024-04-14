#! /bin/bash

if [ -e "hello.txt" ]; then
	rm hello.txt
fi

echo "Hello from process id=$$" >hello.txt
echo "Args: $@" >>hello.txt

sleep 2

# Crash half the time
exit ($RANDOM % 2)
