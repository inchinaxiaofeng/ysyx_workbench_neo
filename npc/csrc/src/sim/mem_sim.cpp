/**
 * 当前文件负责存放"仿真MEM"的包装器代码.
 * "仿真MEM"是一种最小化的, 易于在仿真环境中使用的一系列代码.
 * 这个模块的操作对应与MarCore的`top.sim`的子模块.
 * 在RTL端的接口定义位于MarCore的`top.io.MEMIO`
 *
 * 这个模块中含有一个单步操作, 负责与MEMIO中的成员进行交互.
 * 返回MEMIO.[iRen, iWen, iReadAddr, iWriteAddr, iByteMask, iWriteData],
 * 并视作Core向一个存储单元发送的信息.
 * 这些信息存储在一个被指定的结构体中.
 * 结构体的定义位于与本文件对应的头文件中,
 * 指针需要调用环境去传递进入, 由当前函数进行操作.
 * 函数还有一个MEMIO.oReadData 传入, 被视作存储单元向Core传递数据.
 *
 * 仿真的时序要求由Core的RTL代码规定.
 * 仿真环境应当满足一种及时读取与写入的Mem.
 *
 * "仿真MEM" 并不是用于模拟一个实际的Mem,
 * 它负责提供一个最小影响的理想接口逻辑,
 * 以避免对RTL的性能评估(尤其是各种命中率).
 *
 * 但是需要注意的是, 使用这样的环境,
 * 意味着缓存未命中带来的周期惩罚 **将是不存在的**.
 * 这个会严重缩小 对Cache命中率提升带来性能提升的增长幅度.
 */

extern VSimTop *sim_top;

bool getSimMem(SimMem *mem) {
  if (NULL == mem) {
    return false;
  }
  mem->iRen = sim_top->io_iRen;
  mem->iWen = sim_top->io_iWen;
  mem->iReadAddr = sim_top->io_iReadAddr;
  mem->iWriteAddr = sim_top->io_iWriteAddr;
  mem->iByteMask = sim_top->io_iByteMask;
  mem->iWriteData = sim_top->io_iWriteData;

  return true;
}

void setSimMem(FMT_WORD oReadData) {
  sim_top->io_oReadData = oReadData;
  return;
}
