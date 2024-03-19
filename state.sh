#全局变量
MODDIR=${0%/*}

pid="$(pgrep -f 'regularly.d' | grep -v $$)"
[[ -n $pid ]] && {
  kill -9 "$pid"
}

{
  [[ -f "/data/adb/ksu/bin/busybox" ]] && {
    alias crond="/data/adb/ksu/bin/busybox crond"
  }
} || {
  alias crond="\$( magisk --path )/.magisk/busybox/crond"
}

{
  [[ $(du -k "$MODDIR"/data/crond_data | cut -f1) -ne 0 ]] && {
    time=$(cat "$MODDIR"/data/crond_data)
    echo "$time $MODDIR/clear.sh" >"$MODDIR"/data/regularly.d/root
    crond -c "$MODDIR/data/regularly.d"
    sed -i "/^description=/c description=CROND: [定时进程已恢复运行，请等待开始清理···]" "$MODDIR/module.prop"
  }
} || {
  su -c "sh /data/media/0/Android/RegularlyClean/scheduled_tasks/create_crond.sh &>/dev/null"
}
