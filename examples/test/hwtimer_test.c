#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#ifdef RT_USING_HWTIMER

#define TIMER   "timer0"

static rt_err_t timer_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("HT %d\n", rt_tick_get());

    return 0;
}

int hwtimer(int freq, int t)
{
    rt_err_t err;
    rt_hwtimerval_t val;
    rt_device_t dev = RT_NULL;
    rt_tick_t tick;
    rt_hwtimer_mode_t mode;

    t = (t <= 0)? 5 : t;

    if ((dev = rt_device_find(TIMER)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER);
		return -1;
    }

    rt_device_set_rx_indicate(dev, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq=%dhz Fail\n", freq);
        goto EXIT;
    }

    /* 设置定时器超时值 */
    val.sec = t;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    if (rt_device_write(dev, 0, &val, sizeof(val)) != sizeof(val))
    {
        rt_kprintf("SetTime Fail\n");
        goto EXIT;
    }

    /* 周期模式 */
    mode = HWTIMER_MODE_PERIOD;
    err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);
    /* 启动定时器 */
    tick = rt_tick_get();
    err = rt_device_control(dev, HWTIMER_CTRL_START, RT_NULL);
    rt_kprintf("Start Timer> Tick: %d\n", tick);
    rt_kprintf("Sleep %d sec\n", t);
    rt_thread_delay(t*RT_TICK_PER_SECOND);

    /* 停止定时器 */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* 读取计数 */
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT:
    err = rt_device_close(dev);
    rt_kprintf("Close %s\n", TIMER);

    return err;
}

FINSH_FUNCTION_EXPORT(hwtimer, "Test hardware timer");
#endif
