```javascript
try {
    // 1、创建虚拟机
    const vmId = JSCore.create();
    // 2、执行JS代码获取结果
    const result = JSCore.evaluate(vmId, source);
    // 3、释放虚拟机
    JSCore.release(vmId);
} catch (e) {
    console.error(`evaluate javascript code failed: ${  error.message }`);
}
```