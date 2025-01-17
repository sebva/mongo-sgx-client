SGX_COMMONDIR := $(realpath ./sgx_common)

######## Intel(R) SGX SDK Settings ########
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g -D ENCLAVED
        LIB_MONGOC := trusted/lib/libmongoc-static-1.0-debug.a trusted/lib/libbson-static-1.0-debug.a
else
        SGX_COMMON_CFLAGS += -O2 -D ENCLAVED
        LIB_MONGOC := trusted/lib/libmongoc-static-1.0.a trusted/lib/libbson-static-1.0.a
endif

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif

Crypto_Library_Name := sgx_tcrypto

Mongoclient_Cpp_Files := trusted/mongoclient.cpp trusted/MongoDatabase.cpp
Mongoclient_C_Files := trusted/pthread.c trusted/my_wrappers.c trusted/inet_pton_ntop.c trusted/ssl_wrappers.c
Mongoclient_Include_Paths := -IInclude -Itrusted -I./trusted/include -I./trusted/include/libmongoc-1.0 -I./trusted/include/libbson-1.0 -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/libcxx -I$(SGX_COMMONDIR)


Flags_Just_For_C := -Wno-implicit-function-declaration -std=c11
Common_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Mongoclient_Include_Paths) -pthread -fno-builtin-printf -I.
Mongoclient_C_Flags := $(Flags_Just_For_C) $(Common_C_Cpp_Flags)
Mongoclient_Cpp_Flags :=  $(Common_C_Cpp_Flags) -std=c++11 -nostdinc++ -fno-builtin-printf -I.

Mongoclient_Cpp_Flags := $(Mongoclient_Cpp_Flags)  -fno-builtin-printf

Mongoclient_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	$(LIB_MONGOC) trusted/lib/libssl.a trusted/lib/libcrypto.a \
	-Wl,--whole-archive -l$(Trts_Library_Name) -lsgx_tswitchless -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=trusted/mongoclient.lds

Mongoclient_Cpp_Objects := $(Mongoclient_Cpp_Files:.cpp=.o)
Mongoclient_C_Objects := $(Mongoclient_C_Files:.c=.o)

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: mongoclient.so
	@echo "Build enclave mongoclient.so  [$(Build_Mode)|$(SGX_ARCH)] success!"
	@echo
	@echo "*********************************************************************************************************************************************************"
	@echo "PLEASE NOTE: In this mode, please sign the mongoclient.so first using Two Step Sign mechanism before you run the app to launch and access the enclave."
	@echo "*********************************************************************************************************************************************************"
	@echo 


else
all: mongoclient.signed.so
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/app
	@echo "RUN  =>  app [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif


######## mongoclient Objects ########

libc_mock_%.o: $(SGX_COMMONDIR)/libc_mock/%.cpp
	$(CXX) $(Mongoclient_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

libc_mock_%.o: $(SGX_COMMONDIR)/libc_mock/%.c
	$(CC) $(Mongoclient_C_Flags) -c $< -o $@
	@echo "CC  <=  $<"

sgx_cryptoall.o: $(SGX_COMMONDIR)/sgx_cryptoall.cpp
	$(CXX) $(Mongoclient_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

trusted/mongoclient_t.c: $(SGX_EDGER8R) ./trusted/mongoclient.edl
	cd ./trusted && $(SGX_EDGER8R) --trusted ../trusted/mongoclient.edl --search-path ../trusted --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

trusted/mongoclient_t.o: ./trusted/mongoclient_t.c
	$(CC) $(Mongoclient_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

trusted/%.o: trusted/%.cpp
	$(CXX) $(Mongoclient_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

trusted/%.o: trusted/%.c
	$(CC) $(Mongoclient_C_Flags) -c $< -o $@
	@echo "CC  <=  $<"

mongoclient.so: trusted/mongoclient_t.o $(Mongoclient_Cpp_Objects) $(Mongoclient_C_Objects) libc_mock_file_mock.o libc_mock_libc_proxy.o sgx_cryptoall.o
	$(CXX) $^ -o $@ $(Mongoclient_Link_Flags)
	@echo "LINK =>  $@"

mongoclient.signed.so: mongoclient.so
	$(SGX_ENCLAVE_SIGNER) sign -ignore-init-sec-error -key trusted/mongoclient_private.pem -enclave mongoclient.so -out $@ -config trusted/mongoclient.config.xml
	@echo "SIGN =>  $@"
clean:
	rm -f mongoclient.* trusted/mongoclient_t.* $(Mongoclient_Cpp_Objects) $(Mongoclient_C_Objects)
