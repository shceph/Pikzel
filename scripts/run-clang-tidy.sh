#!/bin/bash
# scripts/run-clang-tidy.sh

echo "Running clang-tidy on all project files..."
echo "=============================================="

clang-tidy -p build/ $(find src/ -name "*.cpp")
# clang-tidy -p build/ $(find src/gla/ -name "*.cpp")

clang-tidy -p build/ $(find src/ -name "*.h" -o -name "*.hpp")
# clang-tidy -p build/ $(find src/gla/ -name "*.h" -o -name "*.hpp")

echo "=============================================="
echo "✅ Clang-tidy complete"
