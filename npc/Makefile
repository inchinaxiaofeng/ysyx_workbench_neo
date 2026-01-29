# 指定性参数
## 模块顶层名字
TOPNAME ?= PureSimTop32
## Refrance Module 名字
REF = riscv64-nemu-interpreter-so
## 存放Verilator编译结果的
BUILD_DIR = ./build
## 需要被运行的BIN文件
IMG ?= ???
## 用于打开VCD的软件
GTK = gtkwave
## 包装器环境头文件
INC_PATH += $(NPC_HOME)/config/autoconfig.h
INC_PATH += $(NPC_HOME)/csrc/include/

# Verilator相关
## 可执行文件
EXEC_OUT = $(BUILD_DIR)/$(TOPNAME)

## 传递给Verilog编译器的参数, 用于控制编译行为
VERILATOR_CFLAGS += -MMD --build -cc \
					-O3 --x-assign fast --x-initial fast --noassert -j 20# \
					#--trace --debug

## Verilog / SystemVerilog 代码
### 尽管我添加了sv, 但是请注意, Verilator对SystemVerilog的支持不充分, 请使用sv2v工具转化.
VSRCS += $(shell find $(NPC_HOME)/vsrc -name "*.v" -or -name "*.sv") ## 包装器的代码
### 包装器的头文件
INCFLAGS += $(addprefix -I, $(INC_PATH))
CSRCS += $(shell find $(NPC_HOME)/csrc -name "*.c" -or -name "*.cc" -or -name "*.cpp")

## 需要链接的库. 主要仿真环境使用.
LDFLAGS += -lreadline -ldl -lSDL2
LDFLAGS += -fsanitize=address -fsanitize=leak

## 传递给Verilator编译器的参数, 包含路径和TOP_NAME的定义
## 主要仿真环境使用.
CFLAGS += $(INCLUDE) -DTOP_NAME"\"V$(TOPNAME)\""
CFLAGS += -Wall

## 静态链接和转化, 将Veirlog代码编译成Cpp代码
## 然后将包装器和Vtop一起编译
## 添加 --exe 可以自动添加 CSRC 和 VSRC
$(EXEC_OUT): $(CSRCS) $(VSRCS)
	@rm -rf $(OBJ_DIR)
	verilator $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) \
		$^ \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(addprefix -CFLAGS , $(CFLAGS)) \
		-- Mdir $(OBJ_DIR) --exe -o $(abspath $(EXEC_OUT))

# 执行

EXEC := $(EXEC_OUT) $(EXEC_FLAGS) $(ARGS) $(IMG)

all:
	@echo "Write this Makefile by your self."

run: run-env

sim: $(EXEC_OUT)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!

include ../Makefile


