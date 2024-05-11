## 项目概述

本项目提供了一个用于处理 CAN FD（Controller Area Network Flexible Data-rate）系统的 C++ 库。以下是库中各个模块的功能概述：

## 工具模块

- **debug_tool**: 通过宏定义，提供了分级的 debug 输出，用于调试和排查问题。
- **math_algorithm**: 封装了数学计算函数，包含在命名空间 `my_algorithm` 中，可用于各种数学计算任务。
- **canfd_tilis**: 定义了 CAN FD 系统的一些默认参数，为其他模块提供了必要的参数设置。

## canfd_frame 模块

本模块封装了消息（`message` 类）和 CAN FD 帧（`canfd_frame` 类）。

- **message 类**: 用于表示信号，封装了随机消息生成函数，并提供了存储在本地文件系统或读取的功能。
- **canfd_frame 类**: 用于表示 CAN FD 帧，封装了 message 并自动计算周期（period）、截止时间（deadline）、payload 尺寸等信息。

## 偏移量和优先级安排模块

在 `canfd_frame` 的基础上，定义了偏移量（offset）和优先级（priority）的安排。

- **offset_arrangement 模块**: 根据论文算法为 `canfd_frame` 分配偏移量，用于确定消息发送的时间。算法来源：《Pushing the limits of CAN - scheduling frames with offsets provides a major performance boost》
- **priority_arrangement 模块**: 根据论文算法为 `canfd_frame` 分配优先级，同时提供了可行性检测功能，用于确定消息发送的优先级和确保系统的实时性。算法来源：《OPTIMAL PRIORITY ASSIGNMENT AND FEASIBILITY OF STATIC PRIORITY TASKS WITH ARBITRARY START TIMES》

## frame_packing 模块

本模块封装了 `packing_scheme` 类和 `FramePacking` 类，它们位于文件 `frame_packing` 中。

- **packing_scheme**: 代表了一组 `canfd_frame`，用于表示对消息集合的打包方案。提供了当前方案的带宽利用率计算功能、启发式算法所需的 fitness 计算功能以及遗传算法生成子代方案的功能。
- **FramePacking 类**: 具体实现了使用遗传算法进行打包的功能。其中 `message_pack` 函数定义了迭代生成结果的方法。算法来源：《A Genetic Algorithm for Packing CAN FD Frame with Real-Time Constraints-IEICE17》
