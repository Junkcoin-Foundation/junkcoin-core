#!/bin/bash
# Junkcoin Core v2 Build Script
# This script builds junkcoin-core-v2 using system libraries

set -e  # Exit on error

echo "========================================="
echo "Junkcoin Core v2 Build Script"
echo "========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "configure.ac" ]; then
    print_error "configure.ac not found. Please run this script from junkcoin-core-v2 directory"
    exit 1
fi

# Step 1: Clean previous build artifacts
print_info "Cleaning previous build artifacts..."
if [ -f "Makefile" ]; then
    make distclean 2>/dev/null || true
fi

# Step 2: Generate configure script
print_info "Generating configure script..."
./autogen.sh

# Step 3: Configure build
print_info "Configuring build..."
print_info "Using system libraries (not depends)"

./configure \
    --disable-tests \
    --disable-bench \
    --with-incompatible-bdb \
    --enable-reduce-exports \
    CXXFLAGS="-O2 -g" \
    CFLAGS="-O2 -g"

# Step 4: Build
print_info "Building junkcoin-core-v2..."
print_info "This may take 10-30 minutes depending on your system..."

# Get number of CPU cores
CORES=$(nproc)
print_info "Using $CORES CPU cores for parallel build"

make -j$CORES

# Step 5: Check if build was successful
if [ -f "src/junkcoind" ] && [ -f "src/junkcoin-cli" ]; then
    print_info "========================================="
    print_info "Build completed successfully!"
    print_info "========================================="
    echo ""
    print_info "Binaries created:"
    echo "  - src/junkcoind       (daemon)"
    echo "  - src/junkcoin-cli    (CLI tool)"
    echo "  - src/junkcoin-tx     (transaction tool)"
    echo "  - src/junkcoin-wallet (wallet tool)"
    echo ""
    print_info "To install system-wide, run: sudo make install"
    echo ""
else
    print_error "Build failed! Check the output above for errors."
    exit 1
fi

