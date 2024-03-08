## Start at 2024-03-07

# Paradise-Lost

「失楽園」になぞらえて、次のステップ、ステージを考えてみる。基本になる開発言語は C/C++ を考えている。

その開発ソースやオブジェクトファイルの利用方法について具体化してみたい。

## 外部と何か行う仕組み

Client Server モデルなのかな。言い方はどうあれ、Client と Server で開発言語の壁を取り除ければ良い。

Client 側の理想は JavaScript であり Server 側は C/C++ を使いたい。

- WebAssembly
- CGI (FCGI)   HTTP(S)
- Socket 通信     TCP/IP
- Shell Script

上記の順番で関心があり、少し調べた限りでは FCGI が最有力候補かもしれない。実現性が他と比べて高いと思うし、既に使われて

いるだろうから。遠回りをするなら、Web サーバを利用しない Socket 通信になる。もう少し考えてみよう。
