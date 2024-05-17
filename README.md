# 音声クロスフェード(拡張編集フィルタプラグイン)

- [ダウンロードはこちら](../../releases/)
- AudioCrossFade.eefをpluginsフォルダに配置してください

- このプラグインを読み込むためにはpatch.aul r43_ss_58以降が必要です https://scrapbox.io/nazosauna/patch.aul

- シーンチェンジのようにオブジェクトの開始点に並べることで音声のクロスフェードを行います。
![image](https://github.com/nazonoSAUNA/AudioCrossFade.eef/assets/99536641/6513eb99-c80b-46df-93d6-f6039994b8b3)

※シーンチェンジでは1個目の中間点で合わせる並べ方（参考 https://scrapbox.io/aviutl/%E3%82%B7%E3%83%BC%E3%83%B3%E3%83%81%E3%82%A7%E3%83%B3%E3%82%B8 ）もありますが、本プラグインでは非対応です。

# 問題点
- クロス前側の処理に音声ディレイが混ざっている場合に正常に処理できません。音声ディレイの処理方法の関係上、対応は難しそうです


# 開発者向け
- aviutl_exedit_sdk：ほぼhttps://github.com/nazonoSAUNA/aviutl_exedit_sdk/tree/efpip を使用しています
