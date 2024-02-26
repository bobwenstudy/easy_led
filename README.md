# 简介

在嵌入式裸机开发中，经常有LED灯效控制的需求，GitHub上暂时没找到合适的项目，所以开发了一个LED灯效控制驱动。本项目地址：[bobwenstudy/easy_led (github.com)](https://github.com/bobwenstudy/easy_led)。

嵌入式开发板上通常有1个或多个LED灯，经常会有LED灯效控制的需求，如控制LED1亮500ms，灭100ms，每隔10s闪烁5次；或者LED1和LED2交替闪烁，LED1亮300ms，LED2亮200ms，闪缩3次后结束等需求。一般都是开发人员自己临时写一些定时器，但是多少体验不是很好，通过本项目可以很好的实现这些管理需求。




# 代码结构

代码结构如下所示：

- **eled**：驱动库，完成EasyLed管理。
- **example_user.c**：跟随系统时间显示灯效（日志打印来测试）。
- **example_test.c**：模拟一些场景的LED事件，对驱动进行测试。
- **main.c**：程序主入口，配置进行测试模式函数用户交互模式。
- **build.mk**和**Makefile**：Makefile编译环境。
- **README.md**：说明文档
- **test_timer.c**：EasyLed所需的软件模拟定时器，实际项目改成自己项目的timer。

```shell
easy_led
 ├── eled
 │   ├── eled.c
 │   └── eled.h
 ├── build.mk
 ├── example_user.c
 └── example_test.c
 ├── main.c
 ├── Makefile
 ├── README.md
 ├── test_timer.h
 └── test_timer.c
```







# 使用说明

## 使用简易步骤

Step1：定义LED_ID、LED闪烁参数和LED数组。注意需要给每一个LED灯实例配置好定时器对象，每个LED灯需要一个定时器来管理其闪烁行为。

```c
typedef enum
{
    USER_LED_RED = 0,
    USER_LED_GREEN,
    USER_LED_BLUE,
    USER_LED_MAX,
} user_led_t;

/* User defined settings */
static const eled_led_param_t test_param_0 = ELED_PARAMS_INIT(200, 200, 5, 0, 0);
static const eled_led_param_t test_param_1 = ELED_PARAMS_INIT(800, 200, 3, 0, 0);
static const eled_led_param_t test_param_2 = ELED_PARAMS_INIT(1000, 2000, 0, 0, 0);


static void test_timer_timeout(void *arg)
{
    // printf("test_timer_timeout()\n");
    eled_process_next_state(arg);
}

static eled_led_t led_red;
static eled_led_t led_green;
static eled_led_t led_blue;

struct test_timer timer_red = {NULL, 0, &led_red, test_timer_timeout};
struct test_timer timer_green = {NULL, 0, &led_green, test_timer_timeout};
struct test_timer timer_blue = {NULL, 0, &led_blue, test_timer_timeout};

static eled_led_t led_red = ELED_LED_INIT(USER_LED_RED, &timer_red);
static eled_led_t led_green = ELED_LED_INIT(USER_LED_GREEN, &timer_green);
static eled_led_t led_blue = ELED_LED_INIT(USER_LED_BLUE, &timer_blue);
```

Step2：实现`eled_start_timer()`和`eled_stop_timer()`接口，考虑到LED业务场景，所有LED切换都用定时器来做，移植时需要对接自身的定时器。

```c

void eled_start_timer(struct eled_led *led, uint16_t time)
{
    struct test_timer *timer = led->timer_handle;

    // printf("eled_start_timer(), time: %d\n", time);
    test_timer_start(timer, time);
}

void eled_stop_timer(struct eled_led *led)
{
    struct test_timer *timer = led->timer_handle;
    // printf("eled_stop_timer()");
    test_timer_stop(timer);
}
```

Step3：初始化LED驱动，注册`prv_led_set_state`接口，用于用户层实现LED真实的点亮和关闭行为。

```c
eled_init(prv_led_set_state);
```

Step4：按需启动不同灯的不同闪烁参数。之后LED灯就会按照参数回调注册的`prv_led_set_state()`。

```c
eled_start(&led_red, &test_param_0);
eled_start(&led_green, &test_param_1);
eled_start(&led_blue, &test_param_2);
```

具体可以参考`example_user.c`和`example_test.c`的实现。





## 结构体说明

### LED配置参数结构体说明-eled_led_param_t

LED每个灯效可以总结为如下参数。

| 名称              | 说明                |
| ----------------- | ------------------- |
| time_active       | LED亮持续时间       |
| time_inactive     | LED灭持续时间       |
| blink_cnt         | LED闪烁次数         |
| time_repeat_delay | LED循环执行等待时间 |
| is_repeat         | LED灯效是否循环执行 |



```c
typedef struct eled_led_param
{
    uint16_t time_active; /*!< LED active time in milliseconds */

    uint16_t time_inactive; /*!< LED inactive time in milliseconds */

    uint16_t blink_cnt; /*!< Total blink cnt. */

    uint16_t time_repeat_delay; /*!< LED repeat delay time in milliseconds */

    uint8_t is_repeat; /*!< Need Repeat or not */
} eled_led_param_t;
```





### LED控制结构体说明-eled_led_t

每个LED灯有一个管理结构体，用于管理该灯的灯效行为。

| 名称              | 说明                                                        |
| ----------------- | ----------------------------------------------------------- |
| timer_handle      | 定时器对象，每个LED灯需要用一个定时器管理其行为             |
| led_id            | 用于表示LED灯                                               |
| blink_reserve_cnt | 当前闪烁剩余次数                                            |
| is_in_process     | LED灯效是否正在执行                                         |
| state             | 当前LED灯的亮灭状态                                         |
| param             | LED灯效时间参数，指向eled_led_param_t，可以绑定不同灯效行为 |





```c
typedef struct eled_led
{
    void *timer_handle; /*!< User for timer manager */

    uint16_t led_id; /*!< Current process led id */

    uint8_t blink_reserve_cnt; /*!< Current led blink reserve cnt */
    uint8_t is_in_process : 4; /*!< is in process */
    uint8_t state : 4;         /*!< Current led state */

    eled_led_param_t param;
} eled_led_t;
```





### LED驱动管理结构体-eled_t

LED驱动需要管理LED灯设置状态回调接口。

| 名称         | 说明                    |
| ------------ | ----------------------- |
| set_state_fn | LED灯状态设置的回调接口 |



```c
typedef struct eled
{
    eled_set_state_fn set_state_fn; /*!< Pointer to set state function */
} eled_t;
```



## 操作API



### 核心API

主要的就是初始化和运行停止接口。

```c
int eled_init(eled_set_state_fn set_state_fn);
void eled_process_next_state(eled_led_t *led);
void eled_stop(eled_led_t *led);
void eled_start(eled_led_t *led, const eled_led_param_t *param);
```



### 其他API

一些工具函数，按需使用。

```c
int eled_is_led_in_process(const eled_led_t *led);
```



### 移植定时器实现API

LED灯效驱动是靠定时器实现，移植时需要实现`eled_start_timer()`和`eled_stop_timer()`接口，行为可以参考example来处理。

```c
extern void eled_start_timer(struct eled_led *led, uint16_t time);
extern void eled_stop_timer(struct eled_led *led);
```





# LED灯效核心处理逻辑说明

LED灯本身只有开关两种状态，但是实际项目中经常会加入时间和周期信息。对LED行为进行抽象，可以简化出`eled_led_param_t`里的这些参数出来。



## LED闪烁

一般LED灯的灯效控制就是控制其闪烁，如下图所示就是`blink_cnt=1`(闪烁2次)的效果。

![image-20240226155802109](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240226155802109.png)

如下图所示，是绿灯闪烁2次的效果。灯效参数为`ELED_PARAMS_INIT(100, 200, 1, 0, 0)`。

![image-20240226160317705](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240226160317705.png)



## LED周期闪烁

LED灯效经常需要循环工作，循环启动会有一个`time_repeat_delay`延迟，如下图所示就是`is_repeat=1 and blink_cnt=2`(周期闪烁2次)的效果。

![image-20240226160031576](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240226160031576.png)



下面显示了在测试下的绿灯周期闪烁2次的效果。灯效参数为`ELED_PARAMS_INIT(200, 300, 1, 1000, 1)`。

![image-20240226160517866](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240226160517866.png)



## 组合灯效场景

其实只要同时启动2个灯的灯效，并且将参数配置合理的值即可。





# 测试说明

## 环境搭建

目前测试暂时只支持Windows编译，最终生成exe，可以直接在PC上跑。

目前需要安装如下环境：
- GCC环境，笔者用的msys64+mingw，用于编译生成exe，参考这个文章安装即可。[Win7下msys64安装mingw工具链 - Milton - 博客园 (cnblogs.com)](https://www.cnblogs.com/milton/p/11808091.html)。


## 编译说明

本项目都是由makefile组织编译的，编译整个项目只需要执行`make all`即可。


也就是可以通过如下指令来编译工程：

```shell
make all
```

而后运行执行`make run`即可运行例程，例程默认运行测试例程，覆盖绝大多数场景，从结果上看测试通过。

```shell
PS D:\workspace\github\easy_led> make run
Compiling  : "example_test.c"
Linking    : "output/main.exe"
Building   : "output/main.exe"
Start Build Image.
objcopy -v -O binary output/main.exe output/main.bin
copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
Print Size
   text    data     bss     dec     hex filename
  41864    3260    2644   47768    ba98 output/main.exe
./output/main.exe
Test running
[      0][     0] ID(hex):   0, state: ON, reserve-cnt:   0
[    100][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
Testing test_events_single ................................................. pass
Testing test_events_single_on_zero ......................................... pass
Testing test_events_single_off_zero ........................................ pass
[      0][     0] ID(hex):   0, state: ON, reserve-cnt:   0
[    100][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
[   1300][  1200] ID(hex):   0, state: ON, reserve-cnt:   0
[   1400][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
[   2600][  1200] ID(hex):   0, state: ON, reserve-cnt:   0
[   2700][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
[   3900][  1200] ID(hex):   0, state: ON, reserve-cnt:   0
[   4000][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
[   5200][  1200] ID(hex):   0, state: ON, reserve-cnt:   0
[   5300][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
[   6500][  1200] ID(hex):   0, state: ON, reserve-cnt:   0
[   6600][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
[   7800][  1200] ID(hex):   0, state: ON, reserve-cnt:   0
[   7900][   100] ID(hex):   0, state: OFF, reserve-cnt:   0
Testing test_events_single_repeat .......................................... pass
[      0][     0] ID(hex):   1, state: ON, reserve-cnt:   1
[    100][   100] ID(hex):   1, state: OFF, reserve-cnt:   1
[    300][   200] ID(hex):   1, state: ON, reserve-cnt:   0
[    400][   100] ID(hex):   1, state: OFF, reserve-cnt:   0
Testing test_events_blink .................................................. pass
Testing test_events_blink_on_zero .......................................... pass
Testing test_events_blink_off_zero ......................................... pass
[      0][     0] ID(hex):   1, state: ON, reserve-cnt:   1
[    200][   200] ID(hex):   1, state: OFF, reserve-cnt:   1
[    500][   300] ID(hex):   1, state: ON, reserve-cnt:   0
[    700][   200] ID(hex):   1, state: OFF, reserve-cnt:   0
[   2000][  1300] ID(hex):   1, state: ON, reserve-cnt:   1
[   2200][   200] ID(hex):   1, state: OFF, reserve-cnt:   1
[   2500][   300] ID(hex):   1, state: ON, reserve-cnt:   0
[   2700][   200] ID(hex):   1, state: OFF, reserve-cnt:   0
[   4000][  1300] ID(hex):   1, state: ON, reserve-cnt:   1
[   4200][   200] ID(hex):   1, state: OFF, reserve-cnt:   1
[   4500][   300] ID(hex):   1, state: ON, reserve-cnt:   0
[   4700][   200] ID(hex):   1, state: OFF, reserve-cnt:   0
[   6000][  1300] ID(hex):   1, state: ON, reserve-cnt:   1
[   6200][   200] ID(hex):   1, state: OFF, reserve-cnt:   1
[   6500][   300] ID(hex):   1, state: ON, reserve-cnt:   0
[   6700][   200] ID(hex):   1, state: OFF, reserve-cnt:   0
[   8000][  1300] ID(hex):   1, state: ON, reserve-cnt:   1
Testing test_events_blink_repeat ........................................... pass
Executing 'run: all' complete!
```















