#全局变量
MODDIR=${0%/*}

#等待用户登录
Wait_until_login() {
  # in case of /data encryption is disabled
  while [ "$(getprop sys.boot_completed)" != "1" ]; do
    sleep 1
  done

  # in case of the user unlocked the screen
  while [ ! -d "/sdcard/Android" ]; do
    sleep 1
  done
}
Wait_until_login

#赋权才能正常运行
chmod -R 0777 "$MODDIR"
#删除历史遗留文件
[[ -f "$MODDIR"/script/tmp/Screen_on ]] && {
  rm -rf "$MODDIR"/script/tmp/Screen_on
}

#如果是ksu则判断busybox是否存在
[[ -f "/data/adb/ksud" ]] && {
  #获取ksu的busybox地址
  busybox="/data/adb/ksu/bin/busybox"
  #释放地址
  filepath="/data/adb/busybox"
  #检查必要文件
  crond_check="/data/adb/busybox/crond"
  #检查Busybox并释放
  [[ -f $busybox ]] && {
    [[ ! -d $filepath ]] || [[ ! -f $crond_check ]] && {
      #先删一次保险
      rm -rf "$filepath" &>/dev/null
      #如果没有此文件夹则创建
      mkdir -p "$filepath"
      #存在Busybox开始释放
      "$busybox" --install -s "$filepath"
    }
  }
}

#拉起子sh进程
. "$MODDIR"/script/clear_the_blacklist_functions.sh

#清空log
logd_clear "开机启动完成: [service.sh]"

#拉起主进程
sh "$MODDIR"/initial.sh

#休息一下
sleep 10

#开启APP监测
sh "$MODDIR"/clear.sh &>/dev/null

#开启监控
"$MODDIR"/script/Timeoff.sh
