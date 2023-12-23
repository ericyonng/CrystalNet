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
#include <ConfigExporter/ConfigExporterIni.h>
#include <ConfigExporter/ConfigExporterApp.h>

Int32 ConfigExporter::Run(int argc, char const *argv[])
{
    KERNEL_NS::SmartPtr<ConfigExporterApp, KERNEL_NS::AutoDelMethods::CustomDelete> app = ConfigExporterApp::New_ConfigExporterApp();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<ConfigExporterApp>(ptr);
        ConfigExporterApp::Delete_ConfigExporterApp(p);
        ptr = NULL;
    });

    app->SetArgs(argc, argv);

    auto err = app->Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ConfigExporter, "init fail err:%d"), err);
        return err;
    }

    err = app->Start();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ConfigExporter, "start fail err:%d"), err);
        return err;
    }

    app->WillClose();
    app->Close();

    return Status::Success;
}
