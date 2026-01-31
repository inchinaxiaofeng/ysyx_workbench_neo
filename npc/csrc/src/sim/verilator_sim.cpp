/*
 * 这个文件存放了最基本的包装器代码, 以及与Vcd相关的部分.
 *
 * 包装器分为两部分, 一个是单步执行(并记录波形),
 * 另一个是通过调用各个单步执行的"综合执行".
 *
 * 这个"综合执行"接受运行的步数, 并且在一个循环中反复执行.
 * 综合执行需要按一定顺序, 根据配置情况, 选择执行一些sim操作.
 *
 * 我们同样向外界暴露单步的接口, 因为对于sdb环境来说, 单步执行很重要.
 * 而使用"综合执行"并传递参数为`1`则会固定影响性能, 所以在一些情况下,
 * 调用各个独立的单步执行还是有必要的.
 *
 * 有两种不同的配置可以影响sim.
 * 一种是在传递的时候同步传递的"trace_en".
 * 它只有在开启的情况下才会记录波形.
 * 另一种是遥测数据获取的使能.
 * 遥测接口在RTL中不可能动态开关,
 * 对应于sim环境, 缺乏"随时开始, 随时暂停"的必要性.
 * 所以我们采用全局变量的形式, 避免显示传递.
 * 但是我们并没有使用选择编译去提高性能, 还是选择保留了动态开关可能性.
 * 可以通过设置这些全局变量来控制.
 */

#include <common.h>
#include <sim/verilator_sim.h>
#include <stdio.h>
#include str(VSimTop_H)

VerilatedContext *contextp;

VerilatedVcdC *tfp = NULL;
VSimTop *sim_top = NULL;

bool trace_en = CONFIG_DEFAULT_VCD;
bool tele_diff_essen = false;
bool tele_diff_reg = false;

void sim_init() {
  LogInfo("[SIM] Initializing Verilator simulation environment...");
  contextp = new VerilatedContext;
  sim_top = new VSimTop;
#ifdef CONFIG_VCD_TRACE
  LogInfo("VCD tracing capability : " ANSI_FG_GREEN " ENABLED " ANSI_NONE);
  contextp->traceEverOn(true);
  tfp = new VerilatedVcdC;
  tfp->open(str(VCD_FILE));
  sim_top->trace(tfp, 5);
  Log("Waveform tracing capability ENABLED.");
#else
  LogInfo("[SIM] VCD tracing capability: " ANSI_FG_RED "DISABLED" ANSI_NONE
          " (Not compiled)");
#endif // CONFIG_VCD_TRACE
  LogOK("[SIM] Simulation environment ready. Time to flip those clocks!");
}

void single_step() {
  // 1. Clock Low
  sim_top->clock = 0;
  sim_top->eval();
  TRACE_DUMP(trace_en, tfp, contextp->time());

  // 2. Clock High
  sim_top->clock = 1;
  sim_top->eval();
  TRACE_DUMP(trace_en, tfp, contextp->time());
}

void sim_exec(int num) {
  for (int i = 0; i < num; i++) {
    single_step();
  }
}

void sim_reset(int num) {
  // 1. Init all 0
  sim_top->reset = 0;
  sim_top->clock = 0;
  sim_top->eval();

  // 2. 0 -> 1: 持续n个周期
  sim_top->reset = 1;
  for (int i = 0; i < num; i++) {
    single_step();
  }

  // 3. 1 -> 0: 复位释放，准备执行
  sim_top->reset = 0;
  LogInfo("[SIM] Reset released after % d cycles.\n", num);
}

void sim_final() {
#ifdef CONFIG_VCD_TRACE
  if (tfp) {
    tfp->close();
    LogInfo("[SIM] Waveform file closed and flushed to disk.");
  } else {
    LogWarn("[SIM] tfp closed too early.");
  }
#endif // VCD_TRACE
  uint64_t total_cycles = contextp->time() / 2;
  LogInfo("[SIM] Simulation Statistics:");
  LogInfo("[SIM] - Total Clock Cycles: %lu", total_cycles);
  sim_top->final();
}
