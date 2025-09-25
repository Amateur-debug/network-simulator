# SystemC Path
SYSTEMC_HOME = /home/CONNECT/xchen740/workspace/systemc/build/systemc

# Network-Simulator Path
NIC_SIMULATOR_HOME = /home/CONNECT/xchen740/workspace/network-simulator

# Test Name
TEST_NAME = send_test

# Build Directory
CORE_BUILD_DIR = $(NIC_SIMULATOR_HOME)/build
TEST_BUILD_DIR = $(NIC_SIMULATOR_HOME)/test/$(TEST_NAME)/build

# Include Path
INCLUDE_PATH = $(NIC_SIMULATOR_HOME)/include
INCLUDE_PATH += $(NIC_SIMULATOR_HOME)/test/$(TEST_NAME)

# Source Path
CORE_SRC_PATH = $(NIC_SIMULATOR_HOME)/src
TEST_SRC_PATH = $(NIC_SIMULATOR_HOME)/test/$(TEST_NAME)

# Source files
CORE_SRC_FILES = $(foreach DIR, $(CORE_SRC_PATH), $(wildcard $(DIR)/*.cpp))
TEST_SRC_FILES = $(wildcard $(TEST_SRC_PATH)/*.cpp)

# Object files
CORE_OBJ_FILES = $(patsubst $(CORE_SRC_PATH)/%.cpp, $(CORE_BUILD_DIR)/%.o, $(CORE_SRC_FILES))
TEST_OBJ_FILES = $(patsubst $(TEST_SRC_PATH)/%.cpp, $(TEST_BUILD_DIR)/%.o, $(TEST_SRC_FILES))
ALL_OBJ_FILES = $(CORE_OBJ_FILES) $(TEST_OBJ_FILES)

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20
CXXFLAGS += -DSC_DISABLE_API_VERSION_CHECK
CXXFLAGS += -Wfatal-errors
CXXFLAGS += -c
CXXFLAGS += -I$(SYSTEMC_HOME)/include
CXXFLAGS += $(addprefix -I,$(INCLUDE_PATH))
LDFLAGS = -L$(SYSTEMC_HOME)/lib -lsystemc

# Target executable
TARGET = sim

# Create directories
$(CORE_BUILD_DIR) $(TEST_BUILD_DIR):
	mkdir -p $(CORE_BUILD_DIR)
	mkdir -p $(TEST_BUILD_DIR)

all: $(TARGET)

$(TARGET): $(ALL_OBJ_FILES) | $(CORE_BUILD_DIR) $(TEST_BUILD_DIR)
	$(CXX) $(LDFLAGS) -o $(TEST_BUILD_DIR)/$@ $^

$(CORE_BUILD_DIR)/%.o: $(CORE_SRC_PATH)/%.cpp | $(CORE_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_BUILD_DIR)/%.o: $(TEST_SRC_PATH)/%.cpp | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

run: $(TARGET)
	LD_LIBRARY_PATH=$(SYSTEMC_HOME)/lib:$$LD_LIBRARY_PATH && \
	export LD_LIBRARY_PATH && \
	cd $(TEST_BUILD_DIR) && ./sim

clean:
	rm -rf $(TEST_BUILD_DIR) $(CORE_BUILD_DIR)

.PHONY: all run clean create_build_dir