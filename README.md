# ECE_5580_SPG6
For Cryptographic Engineering 

## Build
1) `git clone <repo_url>`
2) In Cryptol REPL, `:generate-foreign-header r5_cpa_kem_keygen.cry` and `:q`
3) Adapt yout XKCP path to the _XKCP_PATH_ flag in Makefile
4) `make`
5) Load _r5_cpa_kem_keygen.cry_ into Cryptol and call the implemented functions with required inputs.
