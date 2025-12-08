#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Run the docker container because wow i don't want all these environments on my machine lol
docker run --rm -v "$SCRIPT_DIR":/app -w /app ruby:latest ruby result.rb
