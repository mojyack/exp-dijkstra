# 離散シミュレーション 実験 レポート
## 目的
最短経路探索プログラムの作成
## 方法
プログラムの作成を以下の手順で行った
1. グラフを読み込む部分を実装する
2. Dijstraのアルゴリズムを理解する
3. Dijstraのアルゴリズムを実装する
4. 性能を評価する

## 手順1 グラフの読み込み
### 目的
最短経路探索プログラムの作成に当たり、まずグラフを定義する部分を実装する。

### 実装
図1.1はグラフを表すクラスと、その初期化関数を含んだコード片である。

図1.1: Graphクラス
```C++
struct Graph {
    size_t              size;
    std::vector<double> data;

    auto at(const size_t r, const size_t c) const -> std::optional<double> {
        if(r >= size || c >= size) {
            return std::nullopt;
        }

        return data[r * size + c];
    }

    static auto from_file(const std::string_view path) -> Graph {
        struct Edge {
            size_t from;
            size_t to;
            double weight;
        };

        auto size     = size_t(0);
        auto directed = false;
        auto edges    = std::vector<Edge>();

        auto file     = std::fstream(path);
        auto line     = std::string();
        auto line_num = size_t(0);
        while(std::getline(file, line)) {
            line_num += 1;
            if(line.empty() || line[0] == '#') {
                continue;
            }
            const auto elms = split_strip(line);
            if(elms[0] == "size") {
                if(elms.size() != 2) {
                    println(line_num, ": invalid input");
                    continue;
                }
                if(const auto r = from_chars<size_t>(elms[1]); !r) {
                    println(line_num, ": failed to parse string ", elms[1]);
                    continue;
                } else {
                    size = r.value();
                }
            } else if(elms[0] == "directed") {
                if(elms.size() != 2) {
                    println(line_num, ": invalid input");
                    continue;
                }
                if(elms[1] == "true") {
                    directed = true;
                } else if(elms[1] == "false") {
                    directed = false;
                } else {
                    println(line_num, ": operand to 'directed' must be true or false");
                    continue;
                }
            } else if(elms[0] == "edge") {
                if(elms.size() != 4) {
                    println(line_num, ": invalid input");
                    continue;
                }
                if(const auto r = parse_multi_strings<size_t, size_t, double>({elms[1], elms[2], elms[3]}); !r) {
                    println(line_num, ": failed to parse line");
                    continue;
                } else {
                    auto& v = r.value();
                    edges.emplace_back(Edge{std::get<0>(v), std::get<1>(v), std::get<2>(v)});
                }
            }
        }

        auto graph = Graph{size, std::vector<double>(size * size)};
        std::fill(graph.data.begin(), graph.data.end(), invalid_data);
        for(auto& edge : edges) {
            graph.data[edge.from * size + edge.to] = edge.weight;
            if(!directed) {
                graph.data[edge.to * size + edge.from] = edge.weight;
            }
        }

        return graph;
    }

    static auto from_stdin() -> Graph {
        const auto size = read_stdin<size_t>("size: ");

        auto graph = Graph{size, std::vector<double>(size * size)};
        std::fill(graph.data.begin(), graph.data.end(), invalid_data);

    loop:
        auto a = size_t();
        auto b = size_t();
        if(const auto o = read_value_or_quit<size_t>("from: ", size - 1)) {
            a = o.value();
        } else {
            return graph;
        }
        if(const auto o = read_value_or_quit<size_t>("to: ", size - 1)) {
            b = o.value();
        } else {
            return graph;
        }
        const auto value = read_stdin<double>("weight: ");

        graph.data[a * size + b] = value;
        goto loop;
    }
};
```
`Graph::size`はグラフのノードの数である。  
`Graph::data`は`double`を要素とする一辺が`size`長の二次元配列であり、エッジの重みが記憶されている。なお、存在しないエッジについては、重み`std::numeric_limits<double>::quiet_NaN()`のエッジとして表現した。  
`Graph::from_stdin`は標準入力から情報を読み取り、グラフを構築する。  
`Graph::from_file`はテキストファイルを読み取り、グラフを構築する。そのファイルは以下の図1.2のような書式である。

図1.2 グラフファイル
```
# #で始まる行はコメント
# size ノード数
size     9
# directed (true/false) 有向グラフか否か
directed false
# edge エッジを定義: edge 始点ノード番号 終点ノード番号 重み
edge     1 2 4
edge     0 1 2
edge     0 5 9
edge     0 7 5
edge     1 7 6
edge     7 8 5
edge     5 7 3
edge     5 6 3
edge     2 8 5
edge     3 8 1
edge     2 3 2
edge     3 4 1
edge     4 6 2
edge     6 8 4
edge     6 7 2
edge     4 5 6
```
ちなみに、図1.2は図1.3のグラフを定義している。

図1.3 重みつき無向グラフ^[1]  
![g1.3](https://github.com/mojyack/exp-dijkstra/blob/main/assets/g1.3.png?raw=true)
### 動作確認
標準入力から読み込む場合(図1.4)とファイルから読み込む場合(図1.5)について動作確認を行った

図1.4 標準入力から読み込む場合
```
$ ./a.out
size: 4
from: 1
to: 2
weight: 10
from: 2
to: 3
weight: 15
from:
a=from b=to
a\b  0  1  2  3
  0  X  X  X  X
  1  X  X 10  X
  2  X  X  X 15
  3  X  X  X  X
```

図1.5 ファイルから読み込む場合
```
$ ./a.out input.txt
a=from b=to
a\b  0  1  2  3  4  5  6  7  8
  0  X  2  X  X  X  9  X  5  X
  1  2  X  4  X  X  X  X  6  X
  2  X  4  X  2  X  X  X  X  5
  3  X  X  2  X  1  X  X  X  1
  4  X  X  X  1  X  6  2  X  X
  5  9  X  X  X  6  X  3  3  X
  6  X  X  X  X  2  3  X  2  4
  7  5  6  X  X  X  3  2  X  5
  8  X  X  5  1  X  X  4  5  X
```

## 手順2 Dijstraのアルゴリズムを理解する
### 目的
Dijstraのアルゴリズムを理解し、自分のプログラムに適用できるようにする。
### 方法
テキストやウェブサイト^[2] を参考にしながらDijstra法を勉強し、手動で一通り動かせるようにする。
### 結果
理解の成果として、図1.3のグラフにおいて、ノード1から9の最短経路を求める例を示す。以下、ノードxをnxと表す。

0. n1に最短距離ラベル0を付与する。
1. n1に繋がっていて、かつ最短距離が未確定なのはn2,8,6である。
	1. n2は最短距離ラベルを持っていない。最短距離ラベル2を付与する。前ノードラベル1を付与する。
	2. n8は最短距離ラベルを持っていない。最短距離ラベル5を付与する。前ノードラベル1を付与する。
	3. n6は最短距離ラベルを持っていない。最短距離ラベル9を付与する。前ノードラベル1を付与する。
2. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn2の2である。n2への最短距離を2で確定する。
3. n2に繋がっていて、かつ最短距離が未確定なのはn3,8である。
	1.  n3は最短距離ラベルを持っていない。最短距離ラベル6を付与する。前ノードラベル2を付与する。
	2. n8は最短距離ラベル5を持っている。5<=2+6から、最短距離ラベルを更新しない。
4. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn8の5である。n8への最短距離を5で確定する。
5. n8に繋がっていて、かつ最短距離が未確定なのはn9,7,6である。
	1.  n9は最短距離ラベルを持っていない。最短距離ラベル10を付与する。前ノードラベル8を付与する。
	2.  n7は最短距離ラベルを持っていない。最短距離ラベル7を付与する。前ノードラベル8を付与する。
	3. n6は最短距離ラベル9を持っている。9>5+3から、最短距離ラベルを8へ更新する。前ノードラベルを8へ更新する。
6. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn3の6である。n3への最短距離を6で確定する。
7. n3に繋がっていて、かつ最短距離が未確定なのはn4,9である。
	1.  n4は最短距離ラベルを持っていない。最短距離ラベル8を付与する。前ノードラベル3を付与する。
	9.  n9は最短距離ラベル10を持っている。10<=6+5から、最短距離ラベルを更新しない。	
8. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn7の7である。n7への最短距離を7で確定する。
9. n7に繋がっていて、かつ最短距離が未確定なのはn5,6,9である。
	1.  n5は最短距離ラベルを持っていない。最短距離ラベル9を付与する。前ノードラベル7を付与する。
	2. n6は最短距離ラベル8を持っている。8<=7+3より、最短距離ラベルを更新しない。
	3. n9は最短距離ラベル10を持っている。10<=7+4より、最短距離ラベルを更新しない。
10. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn4の8である。n4への最短距離を8で確定する。
11. n4に繋がっていて、かつ最短距離が未確定なのはn5,9である。
	1.  n5は最短距離ラベル9を持っている。9<=8+1より、最短距離ラベルを更新しない。
	2. n9は最短距離ラベル10を持っている。10>8+1より、最短距離ラベル9へ更新する。前ノードラベルを4へ更新する。
12. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn6の8である。n6への最短距離を8で確定する。
13. n6に繋がっていて、かつ最短距離が未確定なのはn5である。
	1.  n5は最短距離ラベル9を持っている。9<=8+6より、最短距離ラベルを更新しない。
14. 最短距離が確定しておらず、最短距離ラベルを持っていて、かつ最もそれが小さいのはn5,9の9である。n9への最短距離を9で確定する。

以上の操作により、ノード9への最短距離は9であることがわかった。また、n9から前ノードラベルを辿ることで、最短経路が1->2->3->4->9であることがわかった。 
このように、Dijstraのアルゴリズムを手動で動かすことができた。
## 手順3 Dijstraのアルゴリズムを実装する
### 目的
Dijstraのアルゴリズムをプログラムで実装する
### 手段
手順1のプログラムから得られるグラフを引数とし、Dijstraのアルゴリズムを使ってそのグラフの最短経路を求める関数を実装する
### 結果
以下の図3.1が実装した関数である。

図3.1 solve_dijkstra
```C++
auto solve_dijkstra(const size_t start, const Graph& graph) -> void {
    constexpr auto double_max = std::numeric_limits<double>::max();

    struct Label {
        size_t previous     = size_t(-1);
        double total_weight = double_max;
        bool   visited      = false;
    };

    auto labels = std::vector<Label>(graph.size);

    const auto print_route = [&labels, start](const size_t current) {
        auto c = current;
        while(c != start) {
            const auto p = labels[c].previous;
            if(p == size_t(-1)) {
                break;
            }
            print(" <- ", p + 1);
            c = p;
        }
    };

    auto next                  = start;
    labels[start].total_weight = 0;
loop:
    auto  min             = double_max;
    auto  current         = next;
    auto& current_label   = labels[current];
    current_label.visited = true;

    print("current=", current + 1, "(", current_label.total_weight, ")");
    print_route(current);
    print("\n");

    for(auto dest = size_t(0); dest < graph.size; dest += 1) {
        auto& dest_label = labels[dest];
        if(dest_label.visited) {
            continue;
        }
        const auto weight = graph.at(current, dest).value();
        if(!isnan(weight)) {
            println("  dest=", dest + 1);
            println("    total ", current_label.total_weight, "+", weight, " dest ", dest_label.total_weight);

            const auto new_total_weight = current_label.total_weight + weight;
            if(!isnan(weight) && new_total_weight < dest_label.total_weight) {
                dest_label.total_weight = new_total_weight;
                dest_label.previous     = current;
                println("    previous = ", current + 1);
            }
        }
        if(dest_label.total_weight < min) {
            min  = dest_label.total_weight;
            next = dest;
        }
    }
    if(min < double_max) {
        goto loop;
    }

    println("result:");
    for(auto i = size_t(0); i < graph.size; i += 1) {
        auto& label = labels[i];
        print(i + 1, "(", label.total_weight, ")");
        print_route(i);
        print("\n");
    }
}
```

この関数を使って、図1.3のグラフでノード1からの最短経路を求めると以下の図3.2のように出力された。  
図3.2 solve_dijkstraの実行例
```
current=1(0)
  dest=2
    total 0+2 dest 1.8e+308
    previous = 1
  dest=6
    total 0+9 dest 1.8e+308
    previous = 1
  dest=8
    total 0+5 dest 1.8e+308
    previous = 1
current=2(2) <- 1
  dest=3
    total 2+4 dest 1.8e+308
    previous = 2
  dest=8
    total 2+6 dest 5
current=8(5) <- 1
  dest=6
    total 5+3 dest 9
    previous = 8
  dest=7
    total 5+2 dest 1.8e+308
    previous = 8
  dest=9
    total 5+5 dest 1.8e+308
    previous = 8
current=3(6) <- 2 <- 1
  dest=4
    total 6+2 dest 1.8e+308
    previous = 3
  dest=9
    total 6+5 dest 10
current=7(7) <- 8 <- 1
  dest=5
    total 7+2 dest 1.8e+308
    previous = 7
  dest=6
    total 7+3 dest 8
  dest=9
    total 7+4 dest 10
current=4(8) <- 3 <- 2 <- 1
  dest=5
    total 8+1 dest 9
  dest=9
    total 8+1 dest 10
    previous = 4
current=6(8) <- 8 <- 1
  dest=5
    total 8+6 dest 9
current=5(9) <- 7 <- 8 <- 1
current=9(9) <- 4 <- 3 <- 2 <- 1
result:
1(0)
2(2) <- 1
3(6) <- 2 <- 1
4(8) <- 3 <- 2 <- 1
5(9) <- 7 <- 8 <- 1
6(8) <- 8 <- 1
7(7) <- 8 <- 1
8(5) <- 1
9(9) <- 4 <- 3 <- 2 <- 1
```
このように、プログラムを用いて最短経路を求めることができた。

## 手順4 性能の評価
### 目的
実装したプログラムの性能を評価する
### 方法
頂点数nのランダムなグラフを生成し、それを解くのにかかった時間を100回測定する。これをn=10~300に対して行う。  
グラフの生成には以下の図4.1のスクリプトを使用した。  

図4.1 bench.sh
```Bash
#!/bin/zsh
function run() {
    for n in {10..90..10} {100..300..50}; {
        start=$(date +%s.%N)
        repeat 100 python3 gengraph.py $n > graph && echo -e '0\n' | ./a.out graph > /dev/null
        end=$(date +%s.%N)
        echo $n $(($end - $start))
    }
}

run > bench.txt

gnuplot -e '
    set xlabel "size";
    set ylabel "time[sec]";
    set terminal pngcairo;
    set output "/tmp/output.png";
    plot "bench.txt" with linespoints title "time[sec]";
' && imgview /tmp/output.png
```
### 結果
ベンチマークの結果、以下の図4.2のリストと、それをプロットした図4.3が得られた。

図4.2 ベンチマーク結果  
```
10 1.6247682571411133
20 1.7002215385437012
30 1.8270468711853027
40 1.9893424510955811
50 2.1896438598632812
60 2.4448790550231934
70 2.7933440208435059
80 3.133056640625
90 3.5103936195373535
100 3.94474196434021
150 6.8083360195159912
200 10.964922666549683
250 16.425987720489502
300 24.572030782699585
```

図4.3 ベンチマーク結果のグラフ  
![g4.3](https://github.com/mojyack/exp-dijkstra/blob/main/assets/g4.3.png?raw=true)
グラフから、計算時間は頂点数に対して指数関数的に増加していくことがわかった。

## 考察
このプログラムが扱える頂点数の限界を考察する。  
12'084MBの使用可能メモリがある状態で様々な頂点数を入力して実験してみたところ、頂点数が35000のとき正常に動き、40000のときメモリの初期化中にOOMキラーに落とされ、45000のときメモリ確保に失敗した。  
この現象を考察する。このプログラムが必要とするメモリはおおよそ、頂点の数を$v$とすると、$v{\times}v{\times}$sizeof(double) バイトである。  
私の環境では sizeof(double)==8 なので、12'084MBのメモリがあれば、最大で$\sqrt{12084000000{\div8}}{\fallingdotseq}38865$個の頂点が確保できることになる。この計算結果は実験結果と一致する。  
なお、限界より多い、頂点40000個分のメモリ確保に成功したのは、OSの楽観的メモリ配置戦略によるものと思われる。  

## 参考文献
[1] 情報工学実験 III 手順書（離散系のシミュレーション）  
[2] Dijkstraのアルゴリズムについて https://nw.tsuda.ac.jp/lec/dijkstra/
