#!/usr/bin/env bash
# Compile and run every test/test_* suite on the host, the way
# PlatformIO's native env does it (test_build_src = yes): all firmware sources except the .ino,
# the host stand-in libraries, Unity, and one suite's test_main.cpp.
# Usage: run-native.sh <checkout> <harness-lib-dir> <unity-src-dir> [sanitize] [define]
set -u
repo=$1; lib=$2; unity=$3; san=${4:-}; def=${5:-HOST_TEST}
flags="-std=gnu++17 -D$def -Wno-deprecated-declarations -g -O0"
[ "$san" = "sanitize" ] && flags="$flags -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer"
inc="-I$repo/Bluetti_ESP32 -I$unity"
for d in "$lib"/*/; do inc="$inc -I$d"; done
out=$(mktemp -d); pass=0; fail=0
gcc -c $([ "$san" = "sanitize" ] && echo "-fsanitize=address,undefined") -I"$unity" "$unity/unity.c" -o "$out/unity.o" || exit 2
for suite in "$repo"/test/test_*/; do
  name=$(basename "$suite")
  srcs=$(ls "$repo"/Bluetti_ESP32/*.cpp "$lib"/*/*.cpp "$suite"*.cpp)
  if ! g++ $flags $inc $srcs "$out/unity.o" -o "$out/$name" 2> "$out/$name.build.log"; then
    echo "BUILD FAIL $name"; tail -20 "$out/$name.build.log"; fail=$((fail+1)); continue
  fi
  "$out/$name" > "$out/$name.log" 2>&1; code=$?
  # the host main() does not return Unity's failure count, so read Unity's summary line too
  if [ "$code" -eq 0 ] && grep -qE '^[0-9]+ Tests 0 Failures' "$out/$name.log"; then
    echo "PASS $name: $(grep -E '^[0-9]+ Tests' "$out/$name.log")"; pass=$((pass+1))
  else
    echo "FAIL $name"; tail -30 "$out/$name.log"; fail=$((fail+1))
  fi
done
echo "suites passed: $pass, failed: $fail"
[ "$fail" -eq 0 ]
