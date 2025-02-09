import { util } from '@kit.ArkTS';
import { JSCore } from '@devzeng/jscore';
import { BusinessError } from '@kit.BasicServicesKit';

@Extend(Text)
function headerStyle() {
  .textAlign(TextAlign.Start)
  .width('100%')
  .padding({ left: 16, right: 16 })
  .fontSize(16)
  .fontColor('#7E8388')
}

@Extend(TextArea)
function inputStyle() {
  .width('100%')
  .borderRadius(0)
  .fontSize(16)
  .fontColor('#020F22')
}

@Extend(Text)
function outputStyle() {
  .textAlign(TextAlign.Start)
  .width('100%')
  .margin({ top: 10 })
  .padding({ left: 16, right: 16 })
  .fontSize(16)
  .fontColor('#020F22')
}

@Extend(Button)
function btnStyle() {
  .fontSize(16)
  .fontWeight(FontWeight.Regular)
  .backgroundColor('#344F9C')
  .width('35%')
}

@Entry
@Component
struct Index {
  @State script1: string = '';
  @State result1: string = '';
  @State script2: string = '';
  @State result2: string = '';

  aboutToAppear(): void {
    this.requestData();
  }

  build() {
    Column() {
      Column() {
        Text('Input: ')
          .headerStyle()

        TextArea({ text: this.script1 })
          .inputStyle()
          .height('25%')
          .margin({ top: 10 })

        TextArea({ text: this.script2 })
          .inputStyle()
          .height('25%')
          .margin({ top: 10 })
      }
      .margin({ top: 10 })
      .width('100%')
      .padding({ left: 10, right: 10 })

      Row() {
        Button('Run1')
          .btnStyle()
          .onClick(() => {
            this.evaluateScript(this.script1).then((result) => {
              console.log(`evaluate script1.js success: ${result}`);
              this.result1 = result;
            }).catch((error: BusinessError) => {
              console.error(`evaluate script1.js failed: ${  error.message }`);
              this.result1 = 'evaluate script1.js failed: ' + error.message;
            });
          })

        Button('Run2')
          .btnStyle()
          .onClick(() => {
            this.evaluateScript(this.script2).then((result) => {
              console.log(`evaluate script2.js success: ${result}`);
              this.result2 = result;
            }).catch((error: BusinessError) => {
              console.error(`evaluate script2.js failed: ${  error.message }`);
              this.result2 = 'evaluate script2.js failed: ' + error.message;
            });
          })
      }
      .padding({ left: 10, right: 10 })
      .margin({ top: 20 })
      .justifyContent(FlexAlign.SpaceAround)
      .alignItems(VerticalAlign.Center)
      .width('100%')

      if (this.result1 || this.result2) {
        Column() {
          Text('Output')
            .headerStyle()

          if (this.result1) {
            Text(`result1: ${this.result1}`)
              .outputStyle()
              .margin({ top: 10 })
          }

          if (this.result2) {
            Text(`result2: ${this.result2}`)
              .outputStyle()
              .margin({ top: 10 })
          }
        }
        .margin({ top: 20 })
        .justifyContent(FlexAlign.Start)
        .alignItems(HorizontalAlign.Start)
      }
    }
    .height('100%')
    .width('100%')
  }

  requestData() {
    this.readScriptContent('script1.js').then((value) => {
      this.script1 = value;
    }).catch((error: BusinessError) => {
      console.error(`load script1.js failed: ${ JSON.stringify(error) }`);
    });

    this.readScriptContent('script2.js').then((value) => {
      this.script2 = value;
    }).catch((error: BusinessError) => {
      console.error(`load script2.js failed: ${ JSON.stringify(error) }`);
    });
  }

  readScriptContent(name: string): Promise<string> {
    return new Promise((resolve, reject) => {
      getContext(this).resourceManager.getRawFileContent(name, (error, rawFile) => {
        if (error) {
          reject(error);
          return;
        }
        const textDecoder = util.TextDecoder.create('utf-8', { ignoreBOM: true });
        const js = textDecoder.decodeToString(rawFile);
        resolve(js);
      });
    });
  }

  evaluateScript(source: string): Promise<string> {
    return new Promise((resolve, reject) => {
      try {
        const id = JSCore.create();
        const result = JSCore.evaluate(id, source);
        JSCore.release(id);
        resolve(result);
      } catch (e) {
        reject(e);
      }
    });
  }
}