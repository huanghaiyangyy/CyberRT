/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include <cyber/cyber.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/message.h>

#include "apa_proto/odometry.pb.h"

void MessageCallback(
    const std::shared_ptr<davinci_adas::apa::odometry::Odometry>& msg) {
  AERROR << "Received message x-> " << msg->x();
  AERROR << "Received message y-> " << msg->y();
  AERROR << "Received message yaw-> " << msg->yaw();
}

int main(int argc, char* argv[]) {
  // init cyber framework
  apollo::cyber::Init(argv[0]);
  // create listener node
  auto listener_node = apollo::cyber::CreateNode("listener");
  // create listener
  AERROR << "Node listener!";
  auto listener_ =
      listener_node->CreateReader<davinci_adas::apa::odometry::Odometry>(
          "/davinci/apa/odometry", MessageCallback);
  apollo::cyber::WaitForShutdown();
  return 0;
}
