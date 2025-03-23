#!/bin/bash

# Check if correct number of arguments provided
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Invalid number of arguments"
    echo "Usage: $0 fileName IP:Port [bufferSize]"
    echo "Example: $0 test.txt 127.0.0.1:8080 1024"
    exit 1
fi

FILE_NAME="$1"
IP_PORT="$2"
BUFFER_SIZE="${3:-}"

#Check file exists
if [ ! -f "$FILE_NAME" ]; then
    echo "Error: File '$FILE_NAME' not found"
    exit 1
fi

#Check IP:Port format
if ! echo "$IP_PORT" | grep -E '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:[0-9]+$' > /dev/null; then
    echo "Error: Invalid IP:Port format. Should be like 127.0.0.1:8080"
    exit 1
fi

run_client() {
    local instance=$1
    echo "Starting client instance $instance"
    if [ -n "$BUFFER_SIZE" ]; then
        ./clientTCP "$FILE_NAME" "$IP_PORT" "$BUFFER_SIZE" &
    else
        ./clientTCP "$FILE_NAME" "$IP_PORT" &
    fi
}

echo "Launching 5 client instances..."
echo "----------------------------"

# EDIT THIS LINE if you want to change number of clients created.
for i in {1..5}; do
    run_client $i
done

# Wait for all background processes to complete
wait