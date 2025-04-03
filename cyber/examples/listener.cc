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

using apollo::cyber::Node;
using apollo::cyber::Reader;
using apollo::cyber::message::RawMessage;

std::string type_name =
    "davinci_adas.apa.fusion.FusionParkingSlotsInfo";  //"davinci_adas.apa.odometry.Odometry";

bool generate_proto = true;
void GenerateProto(const google::protobuf::Descriptor* descriptor,
                   std::string& output) {
  // 先处理枚举定义
  AERROR << "descriptor->enum_type_count():" << descriptor->enum_type_count();
  for (int i = 0; i < descriptor->enum_type_count(); ++i) {
    const auto* enum_desc = descriptor->enum_type(i);
    output += "  enum " + enum_desc->name() + " {\n";
    for (int j = 0; j < enum_desc->value_count(); ++j) {
      const auto* value = enum_desc->value(j);
      output += "    " + value->name() + " = " +
                std::to_string(value->number()) + ";\n";
    }
    output += "  }\n\n";
  }

  output += "message " + descriptor->name() + " {\n";

  // 遍历所有字段
  for (int i = 0; i < descriptor->field_count(); ++i) {
    const google::protobuf::FieldDescriptor* field = descriptor->field(i);

    // 生成字段定义
    std::string field_rule;
    if (field->is_repeated())
      field_rule = "repeated ";
    else if (field->is_optional())
      field_rule = "optional ";

    std::string type_name;
    // 修改字段类型处理逻辑
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
      type_name = field->message_type()->full_name();
    } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
      type_name = field->enum_type()->name();  // 使用枚举类型名称
    } else {
      type_name = field->type_name();
    }

    output += "  " + field_rule + type_name + " " + field->name() + " = " +
              std::to_string(field->number()) + ";\n";

    if (field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
      const auto* enum_desc = field->enum_type();
      output += "  enum " + enum_desc->name() + " {\n";
      for (int j = 0; j < enum_desc->value_count(); ++j) {
        const auto* value = enum_desc->value(j);
        output += "    " + value->name() + " = " +
                  std::to_string(value->number()) + ";\n";
      }
      output += "  }\n\n";
    }
  }

  // 递归处理嵌套消息
  for (int i = 0; i < descriptor->nested_type_count(); ++i) {
    GenerateProto(descriptor->nested_type(i), output);
  }

  output += "}\n\n";
}

void GenerateProtoFile(std::string type_name) {
  if (type_name.empty()) {
    AERROR << "Failed to get message type name";
    return;
  }
  // 通过Protobuf工厂获取消息描述符
  auto* descriptor = apollo::cyber::message::ProtobufFactory::Instance()
                         ->GenerateMessageByType(type_name)
                         ->GetDescriptor();

  // 语法版本
  std::string output = "syntax = \"proto2\";\n\n";

  // 包名
  if (!descriptor->file()->package().empty()) {
    output += "package " + descriptor->file()->package() + ";\n\n";
  }

  // 生成主消息
  GenerateProto(descriptor, output);
  // 将生成的内容写入文件
  std::string file_name = descriptor->file()->name();
  // 只保留文件名部分
  file_name = file_name.substr(file_name.find_last_of('/') + 1);
  std::ofstream proto_file(file_name);
  if (proto_file.is_open()) {
    proto_file << output;
    proto_file.close();
    AINFO << "Generated proto file: " << file_name;
  } else {
    AERROR << "Failed to open file: " << file_name;
  }
}
void MessageCallback(const std::shared_ptr<RawMessage>& raw_msg) {
  // 获取消息类型名称
  if (type_name.empty()) {
    AERROR << "Failed to get message type name";
    return;
  }
  if (generate_proto) {
    GenerateProtoFile(type_name);
    generate_proto = false;
  }
  return;  // 只生成Proto文件，不打印消息内容
  // 通过Protobuf工厂获取消息描述符
  auto* descriptor = apollo::cyber::message::ProtobufFactory::Instance()
                         ->GenerateMessageByType(type_name)
                         ->GetDescriptor();
  if (!descriptor) {
    AERROR << "Failed to get descriptor for type: " << type_name;
    return;
  }

  // 创建动态消息和工厂
  google::protobuf::DynamicMessageFactory factory;
  const google::protobuf::Message* prototype = factory.GetPrototype(descriptor);
  if (!prototype) {
    AERROR << "Failed to create prototype message";
    return;
  }

  // 创建可修改的消息实例
  std::unique_ptr<google::protobuf::Message> message(prototype->New());
  if (!message->ParseFromString(raw_msg->message)) {
    AERROR << "Failed to parse message data";
    return;
  }

  // 使用反射遍历字段
  const google::protobuf::Reflection* reflection = message->GetReflection();
  std::vector<const google::protobuf::FieldDescriptor*> fields;
  reflection->ListFields(*message, &fields);

  AINFO << "Received message of type: " << descriptor->full_name();
  for (const auto& field : fields) {
    std::string value_str;

    // 根据字段类型处理不同数据
    if (field->is_repeated()) {
      // 处理数组类型（示例仅打印首个元素）
      const int count = reflection->FieldSize(*message, field);
      if (count > 0) {
        value_str = "[...] (repeated, size=" + std::to_string(count) + ")";
      }
    } else {
      // 处理不同类型标量字段
      switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
          value_str = std::to_string(reflection->GetInt32(*message, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
          value_str = reflection->GetString(*message, field);
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
          value_str = std::to_string(reflection->GetUInt32(*message, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
          value_str = std::to_string(reflection->GetFloat(*message, field));
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
          value_str = "{...} (sub-message)";
          break;
        // 添加其他类型的处理...
        default: {
          AERROR << "Unsupported field type: " << field->cpp_type();
          value_str = "[Unhandled Type]";
          break;
        }
      }
    }

    AINFO << "  " << field->name() << ": " << value_str
          << " (type: " << field->cpp_type_name() << ")" << field->cpp_type();
  }
}

int main(int argc, char* argv[]) {
  // 检查命令行参数
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <message_type_name>\n";
    std::cerr << "Example: " << argv[0]
              << " davinci_adas.apa.fusion.FusionParkingSlotsInfo\n";
  } else {
    // 从第一个命令行参数获取类型名称
    type_name = argv[1];
  }

  apollo::cyber::Init(argv[0]);
  auto node = apollo::cyber::CreateNode("raw_reader");
  auto reader = node->CreateReader<RawMessage>(
      "/davinci/apa/fusion/fusion_parkingslots", MessageCallback);

  apollo::cyber::WaitForShutdown();
  return 0;
}
