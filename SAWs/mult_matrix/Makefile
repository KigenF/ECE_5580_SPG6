.SUFFIXES: .c .bc

%.bc : %.c
	clang -g0 -O0 -c -emit-llvm $< -o $@
