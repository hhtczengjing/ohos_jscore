import jsvm from 'libjscore.so';

export class JSCore {

  public static create(): number {
    return jsvm.createJsCore();
  }

  public static release(id: number): void {
    jsvm.releaseJsCore(id);
  }

  public static evaluate(id: number, source: string): string {
    return jsvm.evaluateJS(id, source);
  }
}