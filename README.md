# icpc-kit

ICPC用のローカル作業キットです。リポジトリをcloneしたあと、各メンバーのMacで次を実行します。

```bash
source init.sh
check
```

`source init.sh` はこのシェルだけに `ICPC_KIT`, `ICPC_ENV`, `PATH`, `CPLUS_INCLUDE_PATH` を設定します。`.zshrc` などは変更しません。
`CXX` が未設定の場合は、利用可能な `g++-15`, `g++-14`, `g++-13`, Homebrew GCC, `g++` の順に選びます。

## 新しい作業環境

```bash
nw      # template/1 を使って次の workN を作成
nw 1    # template/1
nw 2    # template/2
nw 3    # template/3
```

`nw` は既存の `workN` の最大番号を見て、次の `workN+1` を作ります。各 `workN` には `A` から `I` までの問題ディレクトリが作られます。

```bash
cd workN/A
```

## 通常テスト

手元でコンパイルしたあと、サンプルテストは `oj` で実行します。

```bash
g++ -std=gnu++20 -O2 -Wall -Wextra -Wshadow -DLOCAL main.cpp
oj t -c ./a.out -d test
```

## ランダムテスト

問題ディレクトリで実行します。

```bash
rt
rt 1000
rt -s 1000
rt --timeout 5 1000
```

`rt` は指定回数だけ `randomTest/gen.cpp` で入力を生成し、`main.cpp` と `randomTest/naive.cpp` の出力を比較します。回数を省略した場合は1回だけ実行します。

各実行のタイムアウトはデフォルトで2秒です。変更したい場合は `--timeout 秒数` を指定します。

`rt -s` で失敗ケースを見つけた場合、`test/sample-k.in`, `test/sample-k.out` に保存します。空の `sample-k.in/out` は未使用枠として扱います。

コンパイラは `CXX` で指定できます。`check` で主要オプションとヘッダの対応状況をまとめて確認できます。`rt` はソースが変わっていない場合、`randomTest/.rt/` の既存バイナリを再利用します。`randomTest/.rt/` はバイナリ置き場として使い、失敗ケースのテキストは保存しません。`CXX` がASANをリンクできない環境では、sanitizerなしで再試行します。

## ACL

AtCoder Library を使う場合は、ローカルでは `#include <atcoder/all>` や `#include <atcoder/dsu>` を使えます。提出前に次を実行します。

```bash
ace
```

`ace` は ACL公式 `expander.py` で `main.cpp` を展開し、提出用の `submit.cpp` を作ります。`ac-library/` には ACL公式の `atcoder/`, `expander.py`, `LICENSE` を置いてください。

## 自作ライブラリ

自作ライブラリは `my-library/` から必要なコードをコピーして使います。提出コードから `my-library/` を `#include` しないでください。

## 開発者向けテスト

キット自体を修正した後は、次を実行します。

```bash
env PYTHONDONTWRITEBYTECODE=1 python3 scripts/kit_check/kit_check.py
```

これは `rt`, `nw`, `init.sh`, `check` の実装が壊れていないかを確認します。GitHub Actions でも push / pull request のたびに同じ検査を実行します。
