# ohos_jscore

JavaScriptCore for HarmonyOS.

## 安装

```shell
ohpm i @devzeng/jscore
```

OpenHarmony ohpm 环境配置等更多内容，请参考[如何安装 OpenHarmony ohpm 包](https://ohpm.openharmony.cn/#/cn/help/downloadandinstall)

## 使用

```javascript
// 1、创建虚拟机
const vmId = JSCore.create();
// 2、执行JS代码获取结果
const result = JSCore.evaluate(vmId, source);
// 3、释放虚拟机
JSCore.release(vmId);
```

## 致谢

项目实现来源于：

- 1、[使用JSVM-API实现JS与C/C++语言交互开发流程](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/use-jsvm-process-V5)

- 2、[使用JSVM-API接口创建多个引擎执行JS代码并销毁](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/napi/use-jsvm-runtime-task.md)