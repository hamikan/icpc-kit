# icpc-kit

## 初期設定

```bash
source init.sh
```

## コマンドが使用可能かのテスト

```bash
check
```
最後に `All required checks passed.` と出ればOK

## テンプレートの作成

template/default ディレクトリをコピーして template フォルダに好きな名前のディレクトリを作成する

## 作業ディレクトリ作成コマンド

```bash
nw                  # template/default を使って ICPC/workN を作成
nw -t 2             # template/2 を使って ICPC/workN を作成
nw --template mikan # template/mikan を使って ICPC/workN を作成
nw -n 2026          # template/default を使って ICPC/2026 を作成
nw --name 2025      # template/default を使って ICPC/2025 を作成
nw -n mikan -t 2     # template/2 を使って ICPC/mikan を作成
```

存在しないテンプレートを指定した場合は `template/default` が使われます。
テンプレートは `-t` または `--template` で指定します。
フォルダ名は `-n` または `--name` で指定します。
指定したテンプレートに `test` や `randomTest` がない場合は、`template/default` のものが使われます。
各問題ディレクトリには、配布テストケースを置くための空の `secret` ディレクトリも作成されます。

## 通常テスト

手元でコンパイルしたあと、サンプルテストは `oj` で実行します。

```bash
g++ main.cpp
oj t
```

## ランダムテスト

問題ディレクトリで実行します。

```bash
rt                  # デフォルトでテスト100回、タイムアウト2秒
rt 1000             # 1000回テスト
rt -s 1000          # WAが出たケースで sample.in, sample.out を自動作成
rt --timeout 5 1000 # タイムアウトを設定
```

`コンパイルされたファイル` より `cppファイル` の方が新しければ自動で新しくコンパイルされます。

## 配布テストケース

ICPC模擬・本番・過去問などで配布された入力と出力は、問題ディレクトリの `secret` にそのまま置きます。

```bash
st          # デフォルトで st in out
st in out   # secret/NAME.in と secret/NAME.out をテスト
st _ out    # secret/NAME と secret/NAME.out をテスト
st in _     # secret/NAME.in と secret/NAME をテスト
st -t 5 in out
st --reset  # ペナをリセット
```

`.` 付きの拡張子は指定しません。`_` は拡張子なしを表します。
出力は空白区切りで比較され、WA / TLE / RE はペナに含まれます。CE はペナに含まれません。

## ACL

AtCoder Library を使う場合は、ローカルでは `#include <atcoder/all>` や `#include <atcoder/dsu>` を使えます。提出前に次を実行します。

```bash
ace # acl を expander.py で展開するコマンド
```

ACL公式 `expander.py` で `main.cpp` を展開し、提出用の `submit.cpp` を作ります。

## 自作ライブラリ

自作ライブラリは `lib` で `main.cpp` の `int main()` の直前に挿入します。

```bash
lib seg    # 自作 SegTree ライブラリを main.cpp に挿入
lib UF     # 自作 UnionFind ライブラリを main.cpp に挿入
lib --list # 使えるコマンド一覧を表示
```

ライブラリ名は小文字として扱います。`lib UF` は `uf` として扱われます。

## 開発者向けテスト

キット自体を修正した後は、次を実行します。

```bash
env PYTHONDONTWRITEBYTECODE=1 python3 scripts/kit_check/kit_check.py
```

これは `rt`, `st`, `nw`, `lib`, `init.sh`, `check` の実装が壊れていないかを確認します。
GitHub Actions でも push / pull request のたびに同じ検査を実行します。
