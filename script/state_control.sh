#全局变量
MODDIR=${0%/*}
#寻找上一个定时程序的进程pid
clear_the_blacklist_crond_pid_1="$(pgrep -f 'set_cron.d' | grep -v $$)"
[[ -n $clear_the_blacklist_crond_pid_1 ]] && {
  kill -9 "$clear_the_blacklist_crond_pid_1"
}

#定义变量
{
  [[ -f "/data/adb/ksud" ]] && {
    alias crond="/data/adb/busybox/crond"
  }
} || {
  alias crond="\$( magisk --path )/.magisk/busybox/crond"
}

#读取规则
{
  [[ $(du -k "$MODDIR"/Crond_time | cut -f1) -ne 0 ]] && {
    time=$(cat "$MODDIR"/Crond_time)
    echo "$time $MODDIR/Run_clear.sh" >"$MODDIR"/set_cron.d/root
    crond -c "$MODDIR/set_cron.d"
    sed -i "/^description=/c description=CROND: [定时进程已恢复运行，请等待开始清理···]" "${MODDIR%/script}/module.prop"
  }
} || {
  su -c "sh /data/media/0/Android/clear_the_blacklist/定时任务/Run_cron.sh &>/dev/null"
}
