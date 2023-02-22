​        build job: 'auto_pressure_test', parameters: [booleanParam(name: 'CleanDb',  value: "${CleanDb}")

​        , string(name: 'Branch', value: "${Branch}"), booleanParam(name: 'IsEmptyInvoke',  value: true)], wait: false



build job: 可以调用其他Jenkins任务，wait:false 表示不需要等待

