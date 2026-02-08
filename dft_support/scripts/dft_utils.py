import pandas as pd
import numpy as np
import subprocess
import os
import sys

def load_cplx_data(file_path):
    """Loads complex data from the CSV format used by the dft_frontend."""
    if not os.path.exists(file_path):
        return None
    # No header, two columns: Real, Imag
    df = pd.read_csv(file_path, header=None, names=['Real', 'Imag'])
    return df['Real'].values + 1j * df['Imag'].values

def run_dft_frontend(bin_path, signal_type, freq, output_name, length_secs):
    """Wraps the calling of the dft_frontend binary."""
    cmd = [
        bin_path,
        "-t", signal_type,
        "-f", str(freq),
        "-o", output_name,
        "-l", str(length_secs)
    ]
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running dft_frontend: {result.stderr}")
    return result.returncode == 0

def find_binary():
    """Attempts to find the dft_frontend binary in common build locations."""
    # Look for the binary relative to the common workspace root
    # Adjust based on known environment
    possible_paths = [
        "build/bin/Release/dft_frontend.exe",
        "build/bin/Debug/dft_frontend.exe",
        "build/bin/dft_frontend",
        "../../build/bin/Release/dft_frontend.exe",
        "../../build/bin/Debug/dft_frontend.exe"
    ]
    
    # Try to find the Repo root if possible
    # For now, we assume we are running from the Repo root or a script subfolder
    for p in possible_paths:
        if os.path.exists(p):
            return os.path.abspath(p)
    return None
