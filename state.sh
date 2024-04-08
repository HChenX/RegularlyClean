#全局变量
MODDIR=${0%/*}

. "$MODDIR"/utils.sh

pid="$(pgrep -f 'regularly.d' | grep -v $$)"
[[ -n $pid ]] && {
  for kill_pid in $pid; do
    kill -9 "$kill_pid"
  done
}

{
  [[ $(du -k "$MODDIR"/data/crond_data | cut -f1) -ne 0 ]] && {
    time=$(cat "$MODDIR"/data/crond_data)
    echo -n "$time $MODDIR/clear.sh" >"$crondFile"
    crond -c "$crondPath"
    sed -i "/^description=/c description=CROND: [定时进程已恢复运行，请等待开始清理···]" "$MODDIR/module.prop"
  }
} || {
  su -c "sh $path/scheduled_tasks/create_crond.sh &>/dev/null"
}
