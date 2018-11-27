Flame 编码风格附加说明
--------------------

### 文件命名
* c++ 头文件使用 `.h` 后缀
* c++ source文件使用 `.cc` 后缀
* c++ 头文件只包含一簇相关类（或只包含一个类）的，采用驼峰式命名，如：`Thread.h`；包含多种类（功能不同，但一般相似的才会放在同一个文件）或不包含类定义的，采用下划线分割命名，如：`io_priority.h`
* source文件与头文件命名方式相同

### 文件编码与缩进
* 使用`utf-8`编码
* 每次缩进对应4个空格，务必不要在文本添加"`\t`"

### 头文件
* 头文件标识唯一的定义，使用路径名转换，如下:

```c++
#ifndef FLAME_COMMON_THREAD_THREAD_H
#define FLAME_COMMON_THREAD_THREAD_H

// your code

#endif // FLAME_COMMON_THREAD_THREAD_H
```

第一个FLAME为项目名，COMMON_THRAED为路径即 `/common/thread` （以`src`作为根目录），最后THREAD_H为文件名即`thread.h`

* 压缩头文件标识，如上一个头文件标识，重复了`THREAD`，可以去重为：
```c++
#ifndef FLAME_COMMON_THREAD_H
#define FLAME_COMMON_THREAD_H

#endif
```

### 类
* 类名使用驼峰式命名，如：`TreadPool`
* 类属性和方法采用下划线分割命名，如：`i_am_a_function()`
* 为避免与第三方库冲突，自定义类统一放入命名空间（`namespace`) `flame` 中
* 按访问权限依次放置：`public`, `protected`, `private`
```c++
class ClassName : public Parent {
public:
    ClassName(...short args...) : a(), b() {}

    ClassName(...long args...)
    : a(), b() { little_thing(); }

    ClassName(...args...) : a() {
        // many_thing()
    }

    ClassName(...args...)
    : a(), b() {
        // many_thing()
    }

protected:
    ...

private:
    ...
}
```

### 代码块
* 使用紧凑型编码，如：

```c++
void function() {
    // do something
}

void show_if() {
    if (CONDITION) {

    } else if (CONDITION) {

    } else {

    }
}

void show_while() {
    while (CONDITION) {

    }

    do {

    } while (CONDITION);
}
```

### 命名空间
* 命名空间内容不缩进，例如：

```c++
namespace flame {
namespace osd {

class ObjectStore {
private:
    int a;
public:
    ObjectStore();
};

} // namespace osd
} // namespace flame

```

### 注释
* 为提高编码效率，允许使用中文注释，但注释格式需要遵从以下规范：
    * 中文注释的头部需要多一个"`*`"，例如： "`/**`"，"`//*`"

```c++
/**
 * English
 */

/* English */

// English

/***
 * 中文
 */

/** 中文 */

//* 中文
```

### 操作符
* 取指符和引用符：`*` 和 `&` 靠近类型名

```c++
int* get_pointer(int* a, int* b) {
    int* c;
    int* d;
    return NULL;
}
```

* 一元操作符：紧跟被操作变量

```c++
int count = 0, src = -1, dst;
++count;
count++;
dst = -src;
```

* 多元操作符：二元操作符或三元操作符前后加空格

```c++
int a, b, c;
c = a + b;
c = a > b ? a : b;

if (a > b)
    ...

T& operator = (const T &other);
```
