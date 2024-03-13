MODDIR=${0%/*}

. "$MODDIR"/script/module_utils.sh
{
  [[ -f "/data/adb/ksu/bin/busybox" ]] && {
    alias crond="/data/adb/ksu/bin/busybox crond"
  }
} || {
  alias crond="\$( magisk --path )/.magisk/busybox/crond"
}

logd "初始化完成: [initial.sh]"
{
  [[ -f "$MODDIR"/script/regularly.d/root ]] && {
    crond -c "$MODDIR"/script/regularly.d
    crond_file=$MODDIR/script/regularly.d/root
  }
} || {
  echo "每天7:00到23:00,每隔60分钟运行一次" >"$MODDIR"/crond_time
  echo "*/60 7-23 * * * $MODDIR/script/clear.sh" >"$MODDIR"/script/regularly.d/root
  crond -c "$MODDIR"/script/regularly.d
  crond_file=$MODDIR/script/regularly.d/root
}

{
  [[ $(pgrep -f 'regularly.d' | grep -v $$) != "" ]] && {
    basic_Information
    logd "$(cat "$MODDIR"/crond_time)"
    logd "开始运行: [$crond_file]"
    logd "------------------------------------------------------------"
  }
} || {
  basic_Information
  logd "运行失败！"
  exit 1
}
sh "$MODDIR"/script/clear.sh