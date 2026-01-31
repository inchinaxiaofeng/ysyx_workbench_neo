# =========================================================
# 1. 加载 Kconfig 生成的配置
# =========================================================
## 假设 auto.conf 在 config 目录或 build 目录下，请根据实际路径调整
########## 你的文件列表中有 auto.conf，请确保路径指向它
CONFIG_FILE := $(NPC_HOME)/config/auto.conf 
# 或者如果它在根目录: CONFIG_FILE := auto.conf

# 如果文件存在，则 include 它
ifneq ($(wildcard $(CONFIG_FILE)),)
    include $(CONFIG_FILE)
endif

# =========================================================
# 2. 提取并清洗 Config
# =========================================================
## 默认值，防止 auto.conf 不存在或没配置时的报错
ifndef CONFIG_TOPNAME
    TOPNAME = SimTop
else
    # 关键步骤：去掉 Kconfig 字符串自带的双引号 ""
    # 比如 "PureSimTop32" -> PureSimTop32
    TOPNAME = $(patsubst "%",%,$(CONFIG_TOPNAME))
endif

ifndef CONFIG_REF_SO
	REF_SO = riscv64-nemu-interpreter-so
else
    REF_SO = $(patsubst "%",%,$(CONFIG_REF_SO))
endif

ifndef CONFIG_VCD_FILE
	VCD_FILE = $(abspath ./log/wave/sim_wave.vcd)
else 
	VCD_FILE = $(abspath $(CONFIG_VCD_FILE))
endif


# 构造 Verilator 生成的类名 (自动加 V 前缀)
V_TOPNAME = V$(TOPNAME)

# 指定性参数
## 存放Verilator编译结果的
BUILD_DIR = ./build
## 需要被运行的BIN文件
IMG ?= 
## 用于打开VCD的软件
GTK = gtkwave
## 包装器环境头文件
INC_PATH += $(NPC_HOME)/config/
INC_PATH += $(NPC_HOME)/csrc/include/
INC_PATH += $(NPC_HOME)/build/
### 架构相关
INC_PATH += $(NPC_HOME)/csrc/src/isa/riscv32/
## 放Verilator编译出的东西的
OBJ_DIR = $(NPC_HOME)/obj_dir

# Verilator相关
## 可执行文件
EXEC_OUT = $(BUILD_DIR)/$(TOPNAME)

## 传递给Verilog编译器的参数, 用于控制编译行为
VERILATOR_CFLAGS += -MMD --build -cc \
					-O3 --x-assign fast --x-initial fast --noassert -j 20
## 检查 auto.conf 中是否开启 VCD_TRACE
ifeq ($(CONFIG_VCD_TRACE),y)
	VERILATOR_CFLAGS += --trace
endif
#--debug

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
CFLAGS += $(INCFLAGS) -DTOP_NAME"\"$(V_TOPNAME)\""
CFLAGS += -Wall
## 在 Makefile 中找到 CFLAGS 定义的地方添加
CFLAGS += -D__GUEST_ISA__=riscv32
# =========================================================
# 3. 注入宏定义到 CFLAGS 
# =========================================================
## 传递给 C++ 编译器：
## 1. VSimTop:  作为类名的别名 (直接替换代码中的 VSimTop)
## 2. VSimTop_H: 头文件名字符串 (用于 #include)
CFLAGS += -DVSimTop=$(V_TOPNAME) -DVSimTop_H='"$(V_TOPNAME).h"'
## 3. VCD_FILE: 存储Wave文件的目录
CFLAGS += -DVCD_FILE=$(VCD_FILE)

## 静态链接和转化, 将Veirlog代码编译成Cpp代码
## 然后将包装器和Vtop一起编译
## 添加 --exe 可以自动添加 CSRC 和 VSRC
$(EXEC_OUT): $(CSRCS) $(VSRCS)
	@rm -rf $(OBJ_DIR)
	@mkdir -p $(BUILD_DIR)
	verilator $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) \
		$^ \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(addprefix -CFLAGS , $(CFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(EXEC_OUT))

# 执行
EXEC_FLAGS = -d $(NEMU_HOME)/build/$(CONFIG_REF_SO)
EXEC := $(EXEC_OUT) $(EXEC_FLAGS) $(ARGS) $(IMG)

all:
	@echo "Write this Makefile by your self."

run: run-env
	$(call git_commit, "run NPC")
	-rm -rf $(VCD_FILE)
	$(EXEC)

run-env: sim $(IMG)

sim: $(EXEC_OUT)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!

include ../Makefile


