#!/bin/bash

# Test proxy server by taking diff of curl commands routed
# through proxy vs directly. Use URL's recommended by project
# specs.

# Create directory folder to store result of tests
directory_path="/h/srolfe01/cs112/assignment1/tests"
mkdir -p "$directory_path"

# Launch proxy server
./a.out 9120 & # Run proxy server in background
proxy_pid=$!   # Capture process ID of proxy
sleep 1        # Wait for proxy to start

# Remove results of prior test
rm -rf "${directory_path:?}"/*

# Run curl command and compare diffs
echo "Testing proxy server..."
curl -sS -x comp112-12:9120 http://www.cs.tufts.edu/comp/112/index.html -o "${directory_path}/proxy_output_index"
curl -sS -x comp112-12:9120 http://www.cs.cmu.edu/~prs/bio.html -o "${directory_path}/proxy_output_bio"
curl -sS -x comp112-12:9120 http://www.cs.cmu.edu/~dga/dga-headshot.jpg -o "${directory_path}/proxy_output_headshot"

curl -sS http://www.cs.tufts.edu/comp/112/index.html -o "${directory_path}/direct_output_index"
curl -sS http://www.cs.cmu.edu/~prs/bio.html -o "${directory_path}/direct_output_bio"
curl -sS http://www.cs.cmu.edu/~dga/dga-headshot.jpg -o "${directory_path}/direct_output_headshot"

diff "${directory_path}/proxy_output_index" "${directory_path}/direct_output_index" > "${directory_path}/diff_index"
diff "${directory_path}/proxy_output_bio" "${directory_path}/direct_output_bio" > "${directory_path}/diff_bio"
diff "${directory_path}/proxy_output_headshot" "${directory_path}/direct_output_headshot" > "${directory_path}/diff_headshot"

# Stop proxy server
echo "Stopping proxy server..."
kill $proxy_pid

# Print output message
if [ -s "${directory_path}/diff_index" ]; then
	echo "Index: Failure"
else 
	echo "Index: Success"
fi

if [ -s "${directory_path}/diff_bio" ]; then
        echo "Bio: Failure"
else 
        echo "Bio: Success"
fi

if [ -s "${directory_path}/diff_headshot" ]; then
        echo "Headshot: Failure"
else 
     	echo "Headshot: Success"
fi

