import pytest
import numpy as np
import os
import sys

# Add scripts folder to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../scripts')))
from dft_utils import load_cplx_data, run_dft_frontend, find_binary

BIN_PATH = find_binary()

@pytest.fixture(scope="module")
def dft_bin():
    if not BIN_PATH:
        pytest.fail("dft_frontend binary not found in build directory. Please build the project first.")
    return BIN_PATH

@pytest.mark.parametrize("freq", [100, 330, 1000])
@pytest.mark.parametrize("length_secs", [0.032, 0.1])
def test_fft_vs_numpy(dft_bin, freq, length_secs, tmp_path):
    output_base = os.path.join(tmp_path, f"test_fft_{freq}_{length_secs}")
    
    # Run the frontend
    assert run_dft_frontend(dft_bin, "sin", freq, output_base, length_secs)

    # Load data
    sig_in = load_cplx_data(f"{output_base}_in")
    sig_dft = load_cplx_data(f"{output_base}_dft")

    assert sig_in is not None
    assert sig_dft is not None

    # Calculate reference FFT with same padding logic
    n_fft = len(sig_dft)
    numpy_fft = np.fft.fft(sig_in, n=n_fft, norm='ortho')

    # Compare magnitude and phase (complex values)
    # Using relative tolerance due to single-precision float limitations in C
    np.testing.assert_allclose(sig_dft, numpy_fft, rtol=1e-4, atol=1e-5)

@pytest.mark.parametrize("freq", [440])
def test_ifft_vs_numpy(dft_bin, freq, tmp_path):
    length_secs = 0.05
    output_base = os.path.join(tmp_path, "test_ifft")
    
    # Run the frontend
    assert run_dft_frontend(dft_bin, "sin", freq, output_base, length_secs)

    # Load original input and the reconstructed inverse
    sig_in = load_cplx_data(f"{output_base}_in")
    sig_inv = load_cplx_data(f"{output_base}_inv")

    assert sig_in is not None
    assert sig_inv is not None

    # sig_inv should match sig_in for the first sig_len samples
    # (since the rest is zero-padding from the forward FFT)
    n_original = len(sig_in)
    sig_inv_truncated = sig_inv[:n_original]

    np.testing.assert_allclose(sig_in, sig_inv_truncated, rtol=1e-5, atol=1e-6)
