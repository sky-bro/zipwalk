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
* `-h` `--help` print usage
* `-s` `--save-file` save files

## Thinking / TODO

* [x] in local file header, filename: folders? files? -- check whether ends with '/' or not
* date & time
* [x] base folder
* [ ] zip32/zip64
* [x] getbyte, getword, getdword, getqword(little endian ?)
* human readable? -h
* [x] mkdir, recursively
* [x] file / folder
* other compression method
  * [x] stored
  * [x] deflate
* [ ] summary info
* [ ] encrypted ?
* [ ] .gz/.tar.gz files

## Refs

* [wiki: ZIP (file format)](https://en.wikipedia.org/wiki/ZIP_(file_format))
* [ZIP压缩算法详细分析及解压实例解释](https://www.cnblogs.com/esingchan/p/3958962.html) 详细，从哈夫曼到deflate
* [zip文件结构](https://www.jianshu.com/p/717373dd3188)
* [zip文件解析](https://www.jianshu.com/p/e42b94967503)
* [rfc1951](http://tools.ietf.org/html/rfc1951) (*deflate* format)
