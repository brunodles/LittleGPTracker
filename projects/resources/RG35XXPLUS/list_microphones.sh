#!/bin/bash
# Lists all available microphone (capture) devices on the device

microphones=$(arecord -l 2>/dev/null | grep -E "^card [0-9]+:")

echo "Available microphones:"
if [ -z "$microphones" ]; then
    echo "  No microphones found."
else
    echo "$microphones" | while IFS= read -r line; do
        echo "  $line"
    done
fi
