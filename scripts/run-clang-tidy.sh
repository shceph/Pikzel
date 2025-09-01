#!/bin/bash
# scripts/run-clang-tidy.sh

echo "Running clang-tidy on all project files..."
echo "=============================================="

clang-tidy -p build/ \
	-header-filter='^src/.*' \
	$(find src/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp")

echo "=============================================="
echo "✅ Clang-tidy complete"
