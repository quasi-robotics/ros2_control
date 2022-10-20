cmake_minimum_required(VERSION 3.5.0)
project(joint_limits_interface)

find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(rclcpp REQUIRED)
find_package(urdf REQUIRED)
find_package(joint_limits REQUIRED)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(joint_limits_urdf_test test/joint_limits_urdf_test.cpp)
  target_include_directories(joint_limits_urdf_test PUBLIC include)
  ament_target_dependencies(joint_limits_urdf_test joint_limits rclcpp urdf)
endif()

# Install headers
install(
  DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_dependencies(rclcpp urdf)

ament_package()
