```javascript
// 1、创建虚拟机
const vmId = JSCore.create();
// 2、执行JS代码获取结果
const source = '1 + 1';
const result = JSCore.evaluate(vmId, source);
// 3、释放虚拟机
JSCore.release(vmId);
```