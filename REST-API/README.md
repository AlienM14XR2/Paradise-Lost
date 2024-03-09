## Start at 2024-03-09

# REST API

Web Server (Nginx)、fcgi、C++ を用いて以前実装した ORM を利用してみる。

## 環境構築

## JSON ライブラリ

ライブラリと書いたが、ヘッダファイルだけあればよい。

```
sudo apt install nlohmann-json3-dev
```
/usr/include/nlohmann

上記ディレクトリ以下に必要なヘッダファイルがある。

## fcgi

```
sudo apt install libfcgi-dev spawn-fcgi
```

コンパイルオブションに次のライブラリを加える
```
-lfcgi++ -lfcgi
```

プロセス起動

g++ などでコンパイル済実行可能ファイルがあることが前提になる。下記[YOUR_OBJECT_FILE] がそれに該当する。

e.g.
```
spawn-fcgi -p 9000 -n [YOUR_OBJECT_FILE]
```


## Web Server（Nginx）

インストールに関しては次を参照してほしい。

https://www.digitalocean.com/community/tutorials/how-to-install-nginx-on-ubuntu-20-04-ja

e.g. 
```
/etc/nginx/sites-available/default
```

等で設定している server ディレクティブに次のものを追加する。
私の設定も単なるコピペだ、信用してはいけない。

```
    location /hello_world
    {
        # spawn-fcgi でプロセスを動作させる host:port
        # これは、自分が起動したプロセスのポートにすること。
        fastcgi_pass   127.0.0.1:9000;
        # nginx の FCGI 各種パラメーター群の設定
        fastcgi_param  GATEWAY_INTERFACE  CGI/1.1;
        fastcgi_param  SERVER_SOFTWARE    nginx;
        fastcgi_param  QUERY_STRING       $query_string;
        fastcgi_param  REQUEST_METHOD     $request_method;
        fastcgi_param  CONTENT_TYPE       $content_type;
        fastcgi_param  CONTENT_LENGTH     $content_length;
        fastcgi_param  SCRIPT_FILENAME    $document_root$fastcgi_script_name;
        fastcgi_param  SCRIPT_NAME        $fastcgi_script_name;
        fastcgi_param  REQUEST_URI        $request_uri;
        fastcgi_param  DOCUMENT_URI       $document_uri;
        fastcgi_param  DOCUMENT_ROOT      $document_root;
        fastcgi_param  SERVER_PROTOCOL    $server_protocol;
        fastcgi_param  REMOTE_ADDR        $remote_addr;
        fastcgi_param  REMOTE_PORT        $remote_port;
        fastcgi_param  SERVER_ADDR        $server_addr;
        fastcgi_param  SERVER_PORT        $server_port;
        fastcgi_param  SERVER_NAME        $server_name;
    }
```
設定ファイルを編集したら、必要に応じてサーバを再起動すること。

```
sudo systemctl restart nginx
```
エラーログの監視
```
sudo tail -f /var/log/nginx/error.log
```
curl を利用したアクセス

e.g.
```
curl -i -s -X POST -d 'this is the post content.' http://localhost/[YOUR_LOCATION_URI]
```

これぐらいしか思い出せない。つまずいたら、自分で調べてほしい、未来の『私』よ。ドキュメントは重要だ、必要があれば追記してくれ。
