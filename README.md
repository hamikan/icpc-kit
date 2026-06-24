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

## 作業ディレクトリ作成コマンド

```bash
nw                  # template/default を使って ICPC/workN を作成
nw -t 2             # template/2 を使って ICPC/workN を作成
nw --template 3     # template/3 を使って ICPC/workN を作成
nw -n 2026          # template/default を使って ICPC/2026 を作成
nw --name 2025      # template/default を使って ICPC/2025 を作成
nw -n 2026 -t 2     # template/2 を使って ICPC/2026 を作成
```

存在しないテンプレートを指定した場合は `template/default` が使われます。
テンプレートは `-t` または `--template` で指定します。
フォルダ名は `-n` または `--name` で指定します。

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

これは `rt`, `nw`, `lib`, `init.sh`, `check` の実装が壊れていないかを確認します。
GitHub Actions でも push / pull request のたびに同じ検査を実行します。
