import jsvm from 'libjscore.so';

export class JSCore {

  // 创建JSVM
  public static create(): number {
    return jsvm.createJsCore();
  }

  // 销毁JSVM
  public static release(id: number): void {
    jsvm.releaseJsCore(id);
  }

  // 执行JS代码
  public static evaluate(id: number, source: string): string {
    return jsvm.evaluateJS(id, source);
  }
}