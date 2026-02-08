import os
import sys
from dft_utils import run_dft_frontend, find_binary

def main():
    bin_path = find_binary()
    if not bin_path:
        print("Error: dft_frontend binary not found. Please build the project first.")
        sys.exit(1)

    print(f"Using binary at: {bin_path}")

    # Example 1: Standard sine wave
    run_dft_frontend(bin_path, "sin", 440, "example_440", 0.05)
    
    # Example 2: Higher frequency
    run_dft_frontend(bin_path, "sin", 1200, "example_1200", 0.1)

    print("Test data generation complete.")
    print("Files created: example_440_in, example_440_dft, example_1200_in, example_1200_dft, etc.")

if __name__ == "__main__":
    main()
