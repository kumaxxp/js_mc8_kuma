# プロポでDonkeyCarを操作する
DonkeyCarの操作には今まで、F710を使っていました。
しかし、学習に使うには分解能が低く、テック系イベント会場ではBlueToothや無線LANは混雑してほぼ使えません。
リアルタイム処理が必要なラジコン操作で使うなら、プロポのほうがよいです。
さらに、柏のDonkeyCarのレース、ランクインできなかったのにJumperT16を残念賞でいただいてしまったので、有効活用することにしました。
残念賞にしては豪華すぎやしませんかね？

# ブレッドボードでプロポの入力をArduinoMicroに入れる
ArduinoはHIDに対応しているため、さすだけで入力デバイスとして使用することができます。
nanoは割り込み入力に対応したポートが少ないため、Microを選びます。
近藤科学のページに基板のレイアウトが載っているので、そちらを参考にするほうが早そうです。
一定のニーズがあると思うので、変換基板を作成したいところです。

# ソフトウェア
私が使用するT16の入力は、16個あるのですが、受信機は8chです。ArduinoMicroの割り込み対応のI/O入力が8chなのと、DonkeyCar程度ならそれほど大量の入力は必要ないです。

## DonkeyCarに必要な入力
プロポからの入力は、アナログ的に処理したいステアリングとアクセル。あとは、モード変更ボタン程度。

|名称|プロポ側の入力|説明|
|---|---|---|
|throttle|位置固定のアナログスティック|スロットル。固定できるの一定速度で走れる|
|steering|位置が中央に戻るアナログスティック|ステアリング|
|erase_last_N_records|3ステートボタンを割当|ONトリガで一定のレコードを消去
|toggle_mode|トリガボタンを割当|手動/auto steering/local pilot の切り替え
|emergency_stop|2ステートボタンを割当|非常停止


## T16の入力などの設定の確認
T16は立ち上げるたびに警告出すんですが、ちょっとうるさいです。
音声を消したいところ。
公式の日本語マニュアルはこちら。
[T16 16-Channel Digital Proportional R/C System](https://drive.google.com/file/d/1er3QyV8o2tyAF2WM-N6u1yPzCu1GZfyi/view)
[BETA FLIGHT 日本語化プロジェクト](https://drive.google.com/file/d/1IqqJVVcwfQFbQZFUbpczUuMlbivy2W39/view)

こちらに詳細な解説をしていただけています。

[Jumper T16（Plus/Proも）を初めて使う人のための解説　初期設定～バインドまで](https://appleroid.com/gadget/jumper-t16-setup-and-settle-issue/)

チャンネルのINPUTS設定を参考に調整します。

T16は左右の十字ボタンの名称が決まっていて、それをチャンネルに割り当てます。

|名称|場所|定義した名称|チャンネル|説明|
|---|---|---|---|---|
|Rud|左/水平|-|-|
|Ele|右/垂直|-|-|
|Thr|左/垂直|Thr|CH1|スロットル|
|Ail|右/水平|Steer|CH2|ステアリング|
|SF|左上|emg|CH3|非常停止|
|SH|右上|mode|CH4|モード切替|
|SG|右上|eras|CH5|nレコード消去|

左の水平、右の垂直は今回使用しません。


