path="/data/media/0/Android/RegularlyClean"
logFile="$path"/log.txt
config="$path"/config.ini
crondPath="$MODDIR"/data/regularly.d
crondFile="$MODDIR"/data/regularly.d/root

{
  [[ -f "/data/adb/ksu/bin/busybox" ]] && {
    alias crond="/data/adb/ksu/bin/busybox crond"
  }
} || {
  alias crond="\$( magisk --path )/.magisk/busybox/crond"
}

logd() {
  echo "[$(date '+%g/%m/%d %H:%M')] | $1" >>$logFile
}

logClear() {
  echo "[$(date '+%g/%m/%d %H:%M')] | $1" >$logFile
}

logNewDay() {
  logClear "是新的一天！: [$(date +'%m/%d') 重置记录]"
}

device() {
  logd "品牌: $(getprop ro.product.brand)"
  logd "型号: $(getprop ro.product.model)"
  logd "代号: $(getprop ro.product.device)"
  logd "安卓: $(getprop ro.build.version.release)"
}
