/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2022-10-08 12:57:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <ConfigExporter/ConfigExporter.h>
#include <service/ConfigExporter/service.h>
#include <ConfigExporter/ConfigExporterIni.h>

class ConfigExporterApp : public SERVICE_COMMON_NS::Application
{
    POOL_CREATE_OBJ_DEFAULT_P1(Application, ConfigExporterApp);

protected:
    virtual void _OnMonitorThreadFrame() override
    {
        // 不打印帧状态
    }
};

// TODO:
// 1.禁用日志显示
// 2.扩展日志打印接口 格式:[PROTOGEN]: cpp gen file:%s, success!
// 3.Csharp导出


POOL_CREATE_OBJ_DEFAULT_IMPL(ConfigExporterApp);

Int32 ConfigExporter::Run(int argc, char const *argv[])
{
    KERNEL_NS::SmartPtr<ConfigExporterApp, KERNEL_NS::AutoDelMethods::CustomDelete> app = ConfigExporterApp::New_ConfigExporterApp();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<ConfigExporterApp>(ptr);
        ConfigExporterApp::Delete_ConfigExporterApp(p);
        ptr = NULL;
    });

    return SERVICE_COMMON_NS::ApplicationHelper::Start(app.AsSelf(), SERVICE_NS::ServiceFactory::New_ServiceFactory(), argc, argv, "", s_appIniContent);
}
