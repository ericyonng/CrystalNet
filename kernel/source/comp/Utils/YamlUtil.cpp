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
// Date: 2026-03-20 22:03:26
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/Utils/YamlUtil.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

bool YamlUtil::TryLoadYamlFile(const LibString &path, YAML::Node &yamlNode)
{
    try
    {
        yamlNode = YAML::LoadFile(path.GetRaw());
    }
    catch (std::exception &e)
    {
        CLOG_ERROR_GLOBAL(YamlUtil, "load yaml fail path:%s, exception:%s", path.c_str(), e.what());
        return false;
    }
    catch (...)
    {
        CLOG_ERROR_GLOBAL(YamlUtil, "load yaml fail path:%s, unkonwn exception", path.c_str());
        return false;
    }

    return true;
}

bool YamlUtil::TryLoadYamlContent(const LibString &content, YAML::Node &yamlNode)
{
    try
    {
        yamlNode = YAML::Load(content.GetRaw());
    }
    catch (std::exception &e)
    {
        CLOG_ERROR_GLOBAL(YamlUtil, "load yaml fail content:%s, exception:%s", content.c_str(), e.what());
        return false;
    }
    catch (...)
    {
        CLOG_ERROR_GLOBAL(YamlUtil, "load yaml fail content:%s, unkonwn exception", content.c_str());
        return false;
    }

    return true;
}


KERNEL_END