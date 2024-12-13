import { JSCore } from '../Index';

@Entry
@Component
struct Index {

  aboutToAppear(): void {
    const id = JSCore.create();
    const result = JSCore.evaluate(id, '1 + 1');
    console.log(result);
    JSCore.release(id);
  }

  build() {
    Column() {
      Blank()
    }
    .height('100%')
    .width('100%')
  }
}