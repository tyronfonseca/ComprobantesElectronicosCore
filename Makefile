CXX ?= clang++
CC ?= clang
# Por defecto -O0 -g para desarrollo; usa make BUILD_MODE=release para -O3
BUILD_MODE ?= debug
ifeq ($(BUILD_MODE),release)
    OPT_FLAGS = -O3
else
    # Hardening de libc++ en debug: detecta comparadores inválidos, índices fuera de rango, etc.
    OPT_FLAGS = -O0 -g -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
endif

OPENSSL_PREFIX ?= $(shell brew --prefix openssl@3 2>/dev/null || echo /opt/homebrew/opt/openssl@3)
ifneq ($(wildcard $(OPENSSL_PREFIX)/include),)
    OPENSSL_INCLUDES = -I$(OPENSSL_PREFIX)/include
    OPENSSL_LIBS = -L$(OPENSSL_PREFIX)/lib -lssl -lcrypto
else
    OPENSSL_INCLUDES =
    OPENSSL_LIBS = -lssl -lcrypto
endif

GTEST_DIR = libs/googletest/googletest
CXXFLAGS ?= -std=c++20 $(OPT_FLAGS) -Wall -Wextra -Wno-character-conversion -Wno-sign-compare -fPIC -MMD -MP
INCLUDES = -Iinclude -Ilibs -I$(GTEST_DIR)/include -I$(GTEST_DIR) $(OPENSSL_INCLUDES)

LIB_NAME = libComprobantesElectronicosCore
BUILD_DIR = build
SRC_DIR = src

SRCS = $(SRC_DIR)/c_api/cec_c_api.cpp \
       $(SRC_DIR)/codec/JsonCodec.cpp \
       $(SRC_DIR)/crypto/Pkcs12Signer.cpp \
       $(SRC_DIR)/crypto/XmlCanonicalizer.cpp \
       $(SRC_DIR)/crypto/XadesSigner.cpp \
       $(SRC_DIR)/common/DivisionTerritorial.cpp \
       $(SRC_DIR)/common/Catalogos.cpp \
       $(SRC_DIR)/common/Monedas.cpp \
       $(SRC_DIR)/common/UnidadesMedida.cpp \
       $(SRC_DIR)/common/FormasFarmaceuticas.cpp \
       libs/pugixml.cpp

OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

TEST_SRCS = $(shell find test -name '*.cpp')
TEST_OBJS = $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
GTEST_OBJ = $(BUILD_DIR)/gtest-all.o

# La librería dinámica depende de la plataforma: .dylib en macOS y .so en el resto.
ifeq ($(shell uname -s),Darwin)
    SHARED_LIB = $(BUILD_DIR)/$(LIB_NAME).dylib
else
    SHARED_LIB = $(BUILD_DIR)/$(LIB_NAME).so
endif

all: $(BUILD_DIR)/$(LIB_NAME).a $(SHARED_LIB) $(BUILD_DIR)/CECTests

$(BUILD_DIR)/$(LIB_NAME).so: $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) -shared -fPIC -o $@ $(OBJS) $(OPENSSL_LIBS)

$(BUILD_DIR)/$(LIB_NAME).a: $(OBJS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $(OBJS)

$(BUILD_DIR)/$(LIB_NAME).dylib: $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) -dynamiclib -o $@ $(OBJS) $(OPENSSL_LIBS)

# Regla genérica para compilar cualquier .cpp a .o
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# GoogleTest amalgamado
$(GTEST_OBJ): $(GTEST_DIR)/src/gtest-all.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -Wno-character-conversion -c $< -o $@

# Tests unitarios y golden
$(BUILD_DIR)/CECTests: $(TEST_OBJS) $(OBJS) $(GTEST_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_OBJS) $(OBJS) $(GTEST_OBJ) $(OPENSSL_LIBS) -o $@

test: $(BUILD_DIR)/CECTests
	./$(BUILD_DIR)/CECTests $(if $(FILTER),--gtest_filter="$(FILTER)",)

# Tests con ThreadSanitizer en un directorio aparte (no mezcla objetos con la compilación normal).
# Por defecto solo corre el grupo de hilos; make tsan FILTER='*' ejecuta toda la suite.
tsan:
	$(MAKE) test BUILD_DIR=build_tsan FILTER='$(or $(FILTER),Hilos*)' CXXFLAGS='-std=c++20 -O1 -g -fsanitize=thread -Wall -Wextra -Wno-character-conversion -Wno-sign-compare -fPIC -MMD -MP'

CFLAGS ?= $(OPT_FLAGS) -Wall -Wextra -fPIC

# Ejemplos de la API de C++ y de C
examples: $(BUILD_DIR)/$(LIB_NAME).a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) examples/cpp/FacturaCppExample.cpp $(BUILD_DIR)/$(LIB_NAME).a $(OPENSSL_LIBS) -o $(BUILD_DIR)/FacturaCppExample
	$(CC) -std=c99 $(CFLAGS) -Iinclude -c examples/c_api/FacturaCApiExample.c -o $(BUILD_DIR)/FacturaCApiExample.o
	$(CXX) $(BUILD_DIR)/FacturaCApiExample.o $(BUILD_DIR)/$(LIB_NAME).a $(OPENSSL_LIBS) -o $(BUILD_DIR)/FacturaCApiExample
	@echo "\n--- Ejemplo C++ ---"
	./$(BUILD_DIR)/FacturaCppExample
	@echo "\n--- Ejemplo C ---"
	./$(BUILD_DIR)/FacturaCApiExample

wasm:
	./scripts/build_wasm.sh $(if $(filter 1,$(DEBUG)),--debug,)

wasm-debug:
	./scripts/build_wasm.sh --debug

clean:
	rm -rf $(BUILD_DIR) build_tsan

-include $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all test examples wasm wasm-debug clean
