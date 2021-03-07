# zipwalk

## Build

```shell
mkdir build
cd build
cmake ../src
make
```

## Run

* `zipwalk`: `./zipwalk -f ../samples/123.zip`
* `test-my_inflate`: `./test/test-my_inflate ../samples/123.deflate 123.txt`

## 目录说明

## getopt

* ./zipwalk
* `-t` `--target_directory`, where to put the extracted files
* `-f` `--file` zipfile path
* `-v` `--version` print version number
* `-?` `--help` print usage
* `-h` human readable

## Thinking / TODO

* in local file header, filename: folders? files? -- check whether ends with '/' or not
* date & time
* base folder
* zip32/zip64
* getbyte, getword, getdword, getqword(little endian ?)
* human readable? -h

## Refs

* [wiki: ZIP (file format)](https://en.wikipedia.org/wiki/ZIP_(file_format))
* [ZIP压缩算法详细分析及解压实例解释](https://www.cnblogs.com/esingchan/p/3958962.html) 详细，从哈夫曼到deflate
* [rfc1951](http://tools.ietf.org/html/rfc1951) (*deflate* format)
