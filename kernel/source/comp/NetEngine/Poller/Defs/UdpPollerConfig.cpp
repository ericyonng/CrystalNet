// /*!
//  *  MIT License
//  *  
//  *  Copyright (c) 2020 ericyonng<120453674@qq.com>
//  *  
//  *  Permission is hereby granted, free of charge, to any person obtaining a copy
//  *  of this software and associated documentation files (the "Software"), to deal
//  *  in the Software without restriction, including without limitation the rights
//  *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  *  copies of the Software, and to permit persons to whom the Software is
//  *  furnished to do so, subject to the following conditions:
//  *  
//  *  The above copyright notice and this permission notice shall be included in all
//  *  copies or substantial portions of the Software.
//  *  
//  *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  *  SOFTWARE.
//  * 
//  * Date: 2022-12-26 14:05:00
//  * Author: Eric Yonng
//  * Description: 
// */

#include <pch.h>
// #include <kernel/comp/NetEngine/Poller/Defs/UdpPollerConfig.h>
// #include <kernel/comp/Utils/ContainerUtil.h>

// KERNEL_BEGIN

// POOL_CREATE_OBJ_DEFAULT_IMPL(UdpPollerInstConfig);

// UdpPollerInstConfig::UdpPollerInstConfig(const UdpPollerFeatureConfig *owner, UInt32 id)
// :_id(id)
// ,_handleRecvBytesPerFrameLimit(0)
// ,_handleSendBytesPerFrameLimit(0)
// ,_handleAcceptPerFrameLimit(0)
// ,_maxPieceTimeInMicroseconds(0)
// ,_maxSleepMilliseconds(0)
// ,_maxPriorityLevel(0)
// ,_pollerInstMonitorPriorityLevel(-1)
// ,_bufferCapacity(0)
// ,_sessionRecvPacketSpeedLimit(0)
// ,_sessionRecvPacketSpeedTimeUnitMs(0)
// ,_sessionRecvPacketStackLimit(0)
// ,_owner(owner)
// {

// }

// LibString UdpPollerInstConfig::ToString() const
// {
//     LibString info;

//     info.AppendFormat("poller inst config id:%u, ", _id)
//         .AppendFormat("recv bytes per frame limit:%llu, ", _handleRecvBytesPerFrameLimit)
//         .AppendFormat("send bytes per frame limit:%llu, ", _handleSendBytesPerFrameLimit)
//         .AppendFormat("accept number per frame limit:%llu, ", _handleAcceptPerFrameLimit)
//         .AppendFormat("max piece time in microseconds limit:%llu, ", _maxPieceTimeInMicroseconds)
//         .AppendFormat("max sleep milliseconds limit:%llu, ", _maxSleepMilliseconds)
//         .AppendFormat("max priority level:%d, ", _maxPriorityLevel)
//         .AppendFormat("poller inst monitor priority level:%d, ", _pollerInstMonitorPriorityLevel)
//         .AppendFormat("buffer capacity:%llu, ", _bufferCapacity)
//         .AppendFormat("session recv packet speed:%llu, ", _sessionRecvPacketSpeedLimit)
//         .AppendFormat("session recv packet speed time unit ms:%llu, ", _sessionRecvPacketSpeedTimeUnitMs)
//         .AppendFormat("session recv packet stack limit:%llu, ", _sessionRecvPacketStackLimit)
//         ;

//     return info;
// }

// void UdpPollerInstConfig::Copy(const UdpPollerInstConfig &cfg)
// {
//     _handleRecvBytesPerFrameLimit = cfg._handleRecvBytesPerFrameLimit;
//     _handleSendBytesPerFrameLimit = cfg._handleSendBytesPerFrameLimit;
//     _handleAcceptPerFrameLimit = cfg._handleAcceptPerFrameLimit;
//     _maxPieceTimeInMicroseconds = cfg._maxPieceTimeInMicroseconds;
//     _maxSleepMilliseconds = cfg._maxSleepMilliseconds;
//     _maxPriorityLevel = cfg._maxPriorityLevel;
//     _pollerInstMonitorPriorityLevel = cfg._pollerInstMonitorPriorityLevel;
//     _bufferCapacity = cfg._bufferCapacity;
// }


// POOL_CREATE_OBJ_DEFAULT_IMPL(UdpPollerFeatureConfig);

// UdpPollerFeatureConfig::UdpPollerFeatureConfig(const UdpPollerConfig *owner, Int32 pollerFeature)
// :_pollerFeature(pollerFeature)
// ,_owner(owner)
// {

// }

// UdpPollerFeatureConfig::~UdpPollerFeatureConfig()
// {
//     ContainerUtil::DelContainer(_pollerInstConfigs, [](TcpPollerInstConfig *&cfg){
//         if(cfg)
//             TcpPollerInstConfig::Delete_TcpPollerInstConfig(cfg);
//         cfg = NULL;
//     });
// }

// LibString UdpPollerFeatureConfig::ToString() const
// {
//     LibString info;
//     info.AppendFormat("feature config _pollerFeature:%d", _pollerFeature);

//     for(auto cfg : _pollerInstConfigs)
//         info.AppendFormat("%s\n", cfg->ToString().c_str());

//     return info;
// }

// void UdpPollerFeatureConfig::Copy(const UdpPollerFeatureConfig &cfg)
// {
//     const Int32 cfgQuantity = static_cast<Int32>(cfg._pollerInstConfigs.size());
//     _pollerInstConfigs.resize(cfgQuantity);
//     for(Int32 idx = 0; idx < cfgQuantity; ++idx)
//     {
//         auto oldCfg = cfg._pollerInstConfigs[idx];
//         auto newCfg = UdpPollerInstConfig::New_UdpPollerInstConfig(this, oldCfg->_id);
//         newCfg->Copy(*oldCfg);
//         _pollerInstConfigs[oldCfg->_id - 1] = newCfg;
//     }
// }

// POOL_CREATE_OBJ_DEFAULT_IMPL(UdpPollerConfig);

// UdpPollerConfig::UdpPollerConfig()
// {

// }

// UdpPollerConfig::~UdpPollerConfig()
// {
//     ContainerUtil::DelContainer(_pollerFeatureRefConfig, [](UdpPollerFeatureConfig *&cfg)
//     {
//         if(cfg)
//             UdpPollerFeatureConfig::Delete_UdpPollerFeatureConfig(cfg);
//         cfg = NULL;
//     });
// }

// LibString UdpPollerConfig::ToString() const
// {
//     LibString info;

//     info.AppendFormat("udp poller config: ");

//     for(auto iter : _pollerFeatureRefConfig)
//     {
//         auto featureConfig = iter.second;
//         info.AppendFormat("%s", featureConfig->ToString().c_str());
//     }

//     return info;
// }

// void UdpPollerConfig::Copy(const UdpPollerConfig &cfg)
// {
//     for(auto iter :cfg._pollerFeatureRefConfig)
//     {
//         auto oldCfg = iter.second;
//         auto newCfg = UdpPollerFeatureConfig::New_UdpPollerFeatureConfig(this, oldCfg->_pollerFeature);
//         newCfg->Copy(*oldCfg);

//         _pollerFeatureRefConfig.insert(std::make_pair(oldCfg->_pollerFeature, newCfg));
//     }
// }

// KERNEL_END
