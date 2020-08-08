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
|emergency_stop|3ステートボタンを割当|非常停止


## T16の入力などの設定の確認
T16は立ち上げるたびに警告出すんですが、ちょっとうるさいです。
音声を消したいところ。



