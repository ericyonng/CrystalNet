// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2025-12-19 00:12:16
// Author: Eric Yonng
// Description:

#include "pch.h"
#include "TestYaml.h"

#include <vector>
#include <yaml-cpp/yaml.h>

struct GameServer
{
    std::string Bind;
    Int32 BindPort;
    std::string HostStr;
    std::vector<Int32> Arr;

    KERNEL_NS::LibString ToString() const
    {
        return KERNEL_NS::LibString().AppendFormat("bind:%s, bindPort:%d, host:%s, arr:%s", Bind.c_str(), BindPort, HostStr.c_str(), KERNEL_NS::StringUtil::ToString(Arr, ',').c_str());
    }
};

namespace YAML
{
    // yaml通过convert来实现自定义类的解析
    template<>
    struct convert<GameServer>
    {
        static Node encode(const GameServer& rhs) {
            Node node;
            node["Bind"] = rhs.Bind;
            node["BindPort"] = rhs.BindPort;
            node["HostStr"] = rhs.HostStr;
            node["Arr"] = rhs.Arr;
            return node;
        }

        static bool decode(const Node& node, GameServer& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            rhs.Bind = node["Bind"].as<std::string>();
            rhs.BindPort = node["BindPort"].as<Int32>();
            rhs.HostStr = node["HostStr"].as<std::string>();
            rhs.Arr = node["Arr"].as<std::vector<Int32>>();
            
            return true;
        }
    };
}

void TestYaml::Run()
{
    auto &&progPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    auto testYamlPath = progPath + "test.yaml";
    YAML::Node config = YAML::LoadFile(testYamlPath.c_str());
    auto gs = config["GameServer"].as<GameServer>();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestYaml, "gs:%s"), gs.ToString().c_str());
    
    // auto bindStr = gs["Bind"].as<std::string>();
    // auto bindPort = gs["BindPort"].as<Int32>();
    // auto hostStr = gs["HostStr"].as<std::string>();
}

