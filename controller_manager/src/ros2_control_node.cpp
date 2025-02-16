// Copyright 2020 ROS2-Control Development Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "controller_manager/controller_manager.hpp"
#include "rclcpp/rclcpp.hpp"
#if __has_include("realtime_tools/realtime_helpers.hpp")
#include "realtime_tools/realtime_helpers.hpp"
#else
#include "realtime_tools/realtime_helpers.h"
#endif

using namespace std::chrono_literals;

namespace
{
// Reference: https://man7.org/linux/man-pages/man2/sched_setparam.2.html
// This value is used when configuring the main loop to use SCHED_FIFO scheduling
// We use a midpoint RT priority to allow maximum flexibility to users
int const kSchedPriority = 50;

}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  std::shared_ptr<rclcpp::Executor> executor =
    std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
  std::string manager_node_name = "controller_manager";

  auto cm = std::make_shared<controller_manager::ControllerManager>(executor, manager_node_name);

  RCLCPP_INFO(cm->get_logger(), "update rate is %d Hz", cm->get_update_rate());

  std::thread cm_thread(
    [cm]()
    {
      if (!realtime_tools::configure_sched_fifo(kSchedPriority))
      {
        RCLCPP_WARN(
          cm->get_logger(),
          "Could not enable FIFO RT scheduling policy: %i. Consider setting up your user to do FIFO RT "
          "scheduling. See "
          "[https://control.ros.org/master/doc/ros2_control/controller_manager/doc/userdoc.html] "
          "for details.", (int)errno);
      }

      rclcpp::Rate loop_rate(cm->get_update_rate());

      // for calculating the measured period of the loop
      rclcpp::Time previous_time = cm->now();

      while (rclcpp::ok())
      {
        // calculate measured period
        auto const current_time = cm->now();
        auto const measured_period = current_time - previous_time;
        previous_time = current_time;

        // execute update loop
        cm->read(cm->now(), measured_period);
        cm->update(cm->now(), measured_period);
        cm->write(cm->now(), measured_period);

        loop_rate.sleep();
      }
    });

  executor->add_node(cm);
  executor->spin();
  cm_thread.join();
  rclcpp::shutdown();
  return 0;
}
