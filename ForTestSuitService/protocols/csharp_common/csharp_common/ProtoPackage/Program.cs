// See https://aka.ms/new-console-template for more information

using System.Reflection;
using Autofac;
using Autofac.Extensions.DependencyInjection;
using DotNetty.Transport.Channels;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Serilog;

namespace ProtoPackage;

public class MyApp
{
    private static ILogger<MyApp>? _logger;

    private static ILogger<MyApp> Logger => _logger!;
    
    private static void Main(string[] args)
    {
        // 1.加载配置
        var configBuilder = CreateConfigurationBinder(args);
        var config = configBuilder.Build();
        
        // 2.创建
        try
        {
            var builder = CreateHostBuilder(args, config);
            var host = builder.Build();
            _logger = host.Services.GetRequiredService<ILogger<MyApp>>();
            Logger.LogInformation("my program started.");
            host.Run();
        }
        catch (System.Exception e)
        {
            Logger.LogError("program run failed{Message}", e.Message);
        }
    }
    
    
    private static IHostBuilder CreateHostBuilder(string[] args, IConfiguration config)
    {
        var builder = Host.CreateDefaultBuilder(args);
        
        // 1.autofac 容器
        builder.UseServiceProviderFactory(new AutofacServiceProviderFactory());

        // 2.日志
        builder.ConfigureLogging(logging =>
        {
            logging.ClearProviders();
            logging.AddConsole();
        });

        // 3.服务器配置
        builder.ConfigureAppConfiguration(configurationBuilder =>
        {
            configurationBuilder.AddConfiguration(config);
        });

        // 4.注册服务,组件等资源
        builder.ConfigureServices(RegisterServices);
        
        // 5.配置容器
        builder.ConfigureContainer<ContainerBuilder>(ConfigureContainer);
        
        return builder;
    }
    
    private static void RegisterServices(HostBuilderContext context, IServiceCollection services)
    {
        // 添加服务
        services.AddHostedService<TestService>();
    }

    private static void ConfigureContainer(HostBuilderContext context, ContainerBuilder builder)
    {
        System.Console.WriteLine("ConfigureContainer...");
    }

    private static IConfigurationBuilder CreateConfigurationBinder(string[] args)
    {
        var config = new ConfigurationManager();
        config.AddCommandLine(args)
            .AddEnvironmentVariables();

        return config;
    }
}