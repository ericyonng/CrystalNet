// Author: EricYonng
// Date:2022-11-22
// Brief:

using Google.Protobuf;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace ProtoPackage;

public class TestService : IHostedService
{
    private readonly ILogger<TestService> _logger;
    
    public TestService(ILogger<TestService> logger)
    {
        _logger = logger;
    }
    
    Task IHostedService.StartAsync(CancellationToken cancellationToken)
    {
        if(_logger.IsEnabled(LogLevel.Information))
            _logger.LogInformation("service started.");

        // 测试proto
        var logReq = new LoginReq {Account = "abc"};
        var byteArray = logReq.ToByteArray();
        var newLogin = LoginReq.Parser.ParseFrom(byteArray);
        _logger.LogInformation("{NewLogin}", newLogin);
        
        return Task.CompletedTask;
    }

    Task IHostedService.StopAsync(CancellationToken cancellationToken)
    {
        if(_logger.IsEnabled(LogLevel.Information))
            _logger.LogInformation("service stop.");
        return Task.CompletedTask;
    }
}