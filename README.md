# Blocklist-OPRF

A C++ implementation on "Blocklisted OPRF".

This software is provided for the purpose of anonymous review by Euro S&P reviewers only. See License.md for more details.

## Running Protocol

The protocol has the following dependencies.

### Dependencies:  
- [GMP](https://gmplib.org)
- [NTL](https://libntl.org/)
- [SSL](https://www.openssl.org/)
- [emp-tool](https://github.com/emp-toolkit)

Setup on a fresh Ubuntu instance:
```sh
$ sudo apt update
$ sudo apt install libgmp-dev libssl-dev
$ wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
$ python install.py --deps --tool --ot --sh2pc
$ wget https://libntl.org/ntl-11.5.1.tar.gz
$ gunzip ntl-11.5.1.tar.gz
$ tar xf ntl-11.5.1.tar
$ cd ntl-11.5.1/src
$ ./configure 
$ make
$ make check
$ sudo make install
```

## Test Files 
Test files "test.cpp" are in each protocol's folder "src". 

**Circuit Generation**
In "test.cpp", set #define TEST_Circuit 1
```sh
$ make
$ ./[binaries]
```

**Explicit Check**
In "test.cpp", set #define TEST_Reg 1
```sh
$ make
$ ./run ./[binaries] 12345 [more opts]
```

**Implicit Check**
In "test.cpp", set #define TEST_Auth 1
```sh
$ make
$ ./run ./[binaries] 12345 [more opts]
```

